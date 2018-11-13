#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "DataDefault.h"
#include "ViewInteractor.h"

int pirPin = D7;
int val;

struct Configuration {
  char trigger[50];
  char key[100];
} configuration;

ESP8266WebServer server(80);

void setup() {

  Serial.begin(115200);
  setupWiFi();
  setupAliasHost();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  loadDataDefault();

  ViewInteractor viewInteractor;
  viewInteractor.lookupFiles();

  server.onNotFound([]() {

    ViewInteractor viewInteractor;
    String path = server.uri();
    if (!viewInteractor.isFileRead(path))

      server.send(404, "text/plain", "FileNotFound");
    else {

      File file = viewInteractor.getFileRead(path);
      size_t sent = server.streamFile(file, viewInteractor.getContentType(path));
      file.close();
    }
  });


  server.on("/setting", []() {

    String trigger = server.arg("trigger");
    Serial.println(trigger);
    String key = server.arg("key");
    Serial.println(key);
    strcpy( configuration.trigger, trigger.c_str());
    strcpy( configuration.key, key.c_str());
    DataDefault<Configuration> dataDefault;
    dataDefault.saveObject(configuration);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your Trigger:  <h1>" ;
    content += trigger ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);
  });

  server.on("/vieweeprom", []() {

    DataDefault<Configuration> dataDefault;
    Configuration configuration = dataDefault.loadObject();

    Serial.println( "Read custom object from EEPROM: " );
    Serial.println( configuration.trigger );
    Serial.println( configuration.key);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your trigger:  <h1>" ;
    content += configuration.trigger ;
    content += "</h1>";
    content += configuration.key;
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";

    server.send(200, "text/html", content);
  });

  server.begin();
}

void loop() {

  server.handleClient();

  val = digitalRead(pirPin);
  //low = no motion, high = motion
  if (val == LOW) {
    Serial.println("No motion");
  } else {
    Serial.println("Motion detected  ALARM");
    alertMotion();
  }

  delay(1000);
}

void alertMotion() {

  delay(5000);
  Serial.print("connecting to ");
  const char* host = "maker.ifttt.com";
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/trigger/";
  url += configuration.trigger;
  url +=  "/with/key/";
  url += configuration.key;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("Respond:");
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void setupWiFi() {

  delay(10000);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(30);
  wifiManager.startConfigPortal();
}

void setupAliasHost() {

  const char* host = "khuonvienxanh";
  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loadDataDefault() {

  DataDefault<Configuration> dataDefault;
  configuration = dataDefault.loadObject();
}
