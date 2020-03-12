#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include <ESP8266mDNS.h>
#include <DHT.h>
#include <math.h>

ESP8266WebServer server(80);

String temStr = "";
String humStr = "";
long lastResetDHT = 0;




struct Configuration {

  char mqttServer[30];
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char mqttpath[30];
  char wifiSSID[30];
  char wifiPassword[30];

} configuration;

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(D4, DHT11);

void setup() {

  Serial.begin(115200);

  loadDataDefault();

  configureServer();

  setupWiFi();

}


void loop() {

  MDNS.update();

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

//  updateTriggerServo();


  float tem = dht.readTemperature(); //C
//   Serial.println("Temp:");
//   Serial.println(String(temp).c_str());



  float hum = dht.readHumidity();

  long now = millis();
  if (now - lastResetDHT > 7000) {
    lastResetDHT = now;
    lastResetDHT = 0;
   
     temStr = "";
    humStr = "";
  }


  String temTopicStr = String(configuration.mqttpath) + "tem";

  if (!temStr.equals(String(tem))) {

    temStr = String(tem);
    client.publish(temTopicStr.c_str(), String(tem).c_str(), true);
  }


  String humTopicStr = String(configuration.mqttpath) + "hum";

   if (!humStr.equals(String(hum))) {

    humStr = String(hum);
    client.publish(humTopicStr.c_str(), String(hum).c_str(), true);
  }




}


void setupWiFi() {
  delay(500);
  String host = String(ESP.getChipId());
  //  const char* host = "thietbi";
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  delay(500);

  //  const char* ssid = "khuonvienxanh";
  String ssid = "KVX" + String(ESP.getChipId());

  WiFi.softAP(ssid.c_str());
  IPAddress myIP = WiFi.softAPIP();

  long now = millis();

  while ((millis() - now) < 30000) {

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

    content += "<h1>Topic Path:  " ;
    content += String(configuration.mqttpath) ;
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
    String topicpath = server.arg("topicpath");

    strcpy( configuration.mqttServer, serverString.c_str());
    strcpy( configuration.mqttUser, username.c_str());
    strcpy( configuration.mqttPassword, password.c_str());
    strcpy( configuration.mqttpath, topicpath.c_str());

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

  server.on("/mqtt", handleMQTTRoot);

  server.begin();
}

void handleRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/wifi.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

void handleMQTTRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/index.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

long lastReconnectWifiAttempt = 0;
void loopConnectWifi() {

  long now = millis();
  if (now - lastReconnectWifiAttempt > 15000) {
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
  if (now - lastReconnectMQTTAttempt > 60000) {

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
  if (client.connected()) {
    return;
  }
  client.setServer(configuration.mqttServer, configuration.mqttPort);

  Serial.println(configuration.mqttServer);
  Serial.println(configuration.mqttPort);
  Serial.println(configuration.mqttUser);
  Serial.println(configuration.mqttPassword);
  Serial.println(configuration.mqttpath);

  Serial.println("Connecting to MQTT...");

  if (client.connect("ESP8266Client", configuration.mqttUser, configuration.mqttPassword )) {

    Serial.println("connected");
    String switchTopic = String(configuration.mqttpath) + "switch";
    client.subscribe(switchTopic.c_str(), 0);

    String openValueTopic = String(configuration.mqttpath) + "openvalue";
    client.subscribe(openValueTopic.c_str(), 1);

    String timeTriggerTopic = String(configuration.mqttpath) + "timetrigger";
    client.subscribe(timeTriggerTopic.c_str(), 1);

    String firstPingTopic = String(configuration.mqttpath) + "FirstPing";
    client.publish(firstPingTopic.c_str(), "Hello ");
  } else {

    Serial.print("failed with state ");
    Serial.print(client.state());
  }
}
