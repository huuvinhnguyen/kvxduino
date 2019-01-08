#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

struct Configuration {

  char mqttServer[30];
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char wifiSSID[30];
  char wifiPassword[30];

} configuration;

WiFiClient espClient;
PubSubClient client(espClient);


uint8_t ledPin = 2;

void setup() {

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  loadDataDefault();


  configureServer();

  setupWiFi();
}


void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    if (client.connected()) {

      client.loop();
    } else {

      loopConnectMQTT();
    }
  } else {

    loopConnectWifi();
  }

  server.handleClient();
}

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;

  Serial.print(str);
  if (str.equals("1") ) {
    digitalWrite(ledPin, LOW);

  } else {

    digitalWrite(ledPin, HIGH);
  }

  Serial.println();
  Serial.println("-----------------------");
  //  digitalWrite(ledPin, !digitalRead(ledPin));

}

void setupWiFi() {
  delay(500);
  const char* host = "thietbi";
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  delay(500);

//  const char* ssid = "khuonvienxanh";
  String ssid = "ESP" + String(ESP.getChipId());

  WiFi.softAP(ssid.c_str());
  IPAddress myIP = WiFi.softAPIP();

  long now = millis();

  while ((millis() - now) < 60000) {

    server.handleClient();

  }
}

void loadDataDefault() {

  DataDefault<Configuration> dataDefault;
  configuration = dataDefault.loadObject();
  delay(500);

}

void configureServer() {

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

  server.on("/vieweeprom", []() {

    Serial.println( "Read custom object from EEPROM: " );

    String content = "<html>";
    content += "<body>";

    content += "<h1>User:  " ;
    content += configuration.mqttUser ;
    content += "</h1> <br>";

    content += "<h1>Password:  " ;
    content += configuration.mqttPassword ;
    content += "</h1> <br>";

    content += "<h1>Server:  " ;
    content += configuration.mqttServer ;
    content += "</h1> <br>";

    content += "<h1>Port:  " ;
    content += String(configuration.mqttPort) ;
    content += "</h1> <br>";

    content += "<h1>SSID:  " ;
    content += configuration.wifiSSID ;
    content += "</h1> <br>";

    content += "<h1>Password:  " ;
    content += configuration.wifiPassword ;
    content += "</h1> <br>";

    content += "</body>";
    content += "</html>";

    server.send(200, "text/html", content);
  });


  server.on("/setting", []() {

    String serverString = server.arg("server");
    Serial.println(serverString);
    String username = server.arg("username");
    Serial.println(username);
    String password = server.arg("password");
    String port = server.arg("port");
    strcpy( configuration.mqttServer, serverString.c_str());
    strcpy( configuration.mqttUser, username.c_str());
    strcpy( configuration.mqttPassword, password.c_str());
    configuration.mqttPort = port.toInt();

    DataDefault<Configuration> dataDefault;
    dataDefault.saveObject(configuration);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your file Id:  <h1>" ;
    //      content += fileId ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);

  });

  server.on("/wifisetting", []() {

    String ssid = server.arg("ssid");
    String wifiPass = server.arg("wifiPassword");

    strcpy( configuration.wifiSSID, ssid.c_str());
    strcpy( configuration.wifiPassword, wifiPass.c_str());


    DataDefault<Configuration> dataDefault;
    dataDefault.saveObject(configuration);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your Wifi:  <h1>" ;
    //      content += fileId ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);
    
    connectWifi();

  });
  
  server.on("/", handleRoot);
  server.begin();
}

void handleRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/wifi.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

long lastReconnectWifiAttempt = 0;
void loopConnectWifi() {

  long now = millis();
  if (now - lastReconnectWifiAttempt > 5000) {
    lastReconnectWifiAttempt = now;
    connectWifi();
    if (WiFi.status() == WL_CONNECTED) {
      lastReconnectWifiAttempt = 0;
    }

  }
}

void connectWifi() {
  
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(configuration.wifiSSID, configuration.wifiPassword);
  Serial.print("SSID: ");
  Serial.println(configuration.wifiSSID);
  Serial.print("Password: ");

  Serial.println(configuration.wifiPassword);

  Serial.print("wifi connecting...");
}

long lastReconnectMQTTAttempt = 0;

void loopConnectMQTT() {

  long now = millis();
  if (now - lastReconnectMQTTAttempt > 5000) {

    lastReconnectMQTTAttempt = now;
    // Attempt to connect
    connectMQTT();

    if (client.connected()) {
      lastReconnectMQTTAttempt = 0;
    }
  }
}

void connectMQTT() {

  delay(500);
  client.setServer(configuration.mqttServer, configuration.mqttPort);
  client.setCallback(callback);

  Serial.println(configuration.mqttServer);
  Serial.println(configuration.mqttPort);
  Serial.println(configuration.mqttUser);
  Serial.println(configuration.mqttPassword);

  Serial.println("Connecting to MQTT...");

  if (client.connect("ESP8266Client", configuration.mqttUser, configuration.mqttPassword )) {

    Serial.println("connected");
    client.subscribe("switch", 1);
    client.publish("FirstPing", "Hello ");
  } else {

    Serial.print("failed with state ");
    Serial.print(client.state());
  }
}
