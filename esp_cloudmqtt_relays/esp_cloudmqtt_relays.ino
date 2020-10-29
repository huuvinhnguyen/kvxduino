#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include <ESP8266mDNS.h>
#include "Relay.h"

ESP8266WebServer server(80);
uint8_t relayPin = 13;
uint8_t relayPins[4] = {5, 4, 0, 2};
Relay relay;




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

uint8_t ledPin = 17;


void setup() {

  Serial.begin(115200);
  setupTimeClient();

  loadDataDefault();

  configureServer();

  setupWiFi();

  relay.setup("");

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

  updateTriggerRelay();
  relay.loop([](int count) {
    String switchTopic = String(configuration.mqttpath) + "switchon";
    Serial.println(switchTopic);

    client.publish(switchTopic.c_str(), "done", true);

    Serial.println(count);
  });
}


char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0) {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}

//#include <Servo.h>
//
//Servo myservo;
#define CONTROL_PIN D7
int ser_pos_feeder = 70;
int ser_pos_fishtank = 50;
int pos = 0;    // variable to store the servo position

#include <NTPClient.h>
#include "Timer.h"
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
WatchDog watchDog;

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;
  Serial.print(str);

  String publishTopic = String(configuration.mqttpath) + "switch";

  relay.handleMessage(topic, str);


  String openValueTopic = String(configuration.mqttpath) + "openvalue";
  if (strcmp(topic, openValueTopic.c_str()) == 0) {

    if (str.equals("done")) {
      Serial.println("#done");
    } else {

      ser_pos_fishtank = str.toInt();
      Serial.print("set value open: ");
      Serial.println(ser_pos_fishtank);
      //      client.publish(topic, "done", false);
    }
  }

  String timeTriggerTopic = String(configuration.mqttpath) + "timetrigger";
  if (strcmp(topic, timeTriggerTopic.c_str()) == 0) {

    if (str.equals("done")) {
      Serial.println("#done");
    } else {

      watchDog.setTimeString(str);
      Serial.print("trigger string: ");
      Serial.println(str);
      //      client.publish(topic, "done", false);
    }
  }

  Serial.println("ok");
  Serial.println("-----------------------");
  //  digitalWrite(ledPin, !digitalRead(ledPin));

}

void setupTimeClient() {
  timeClient.begin();
}

void updateTriggerRelay() {
  timeClient.update();

  bool isActive = watchDog.isAlarmAtTime(timeClient.getHours(), timeClient.getMinutes());
  if (isActive) {
    Serial.println("Activate servo");
    activateRelays();
  }
}

void activateServo() {

  digitalWrite(relayPin, HIGH);
  delay(ser_pos_fishtank);
  digitalWrite(relayPin, LOW);
}

void activateRelays() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(relayPins[i], LOW);
    delay(ser_pos_fishtank);
    digitalWrite(relayPins[i], HIGH);
  }

}

void inactivateRelays() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(relayPins[i], HIGH);
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
  client.setCallback(callback);

  Serial.println(configuration.mqttServer);
  Serial.println(configuration.mqttPort);
  Serial.println(configuration.mqttUser);
  Serial.println(configuration.mqttPassword);
  Serial.println(configuration.mqttpath);

  Serial.println("Connecting to MQTT...");

  if (client.connect("ESP8266Client", configuration.mqttUser, configuration.mqttPassword )) {

    Serial.println("connected");
    String switchTopic = String(configuration.mqttpath) + "switch";
    client.subscribe(switchTopic.c_str(), 1);

    String switchonTopic = String(configuration.mqttpath) + "switchon";
    client.subscribe(switchonTopic.c_str(), 1);

    String timeTriggerTopic = String(configuration.mqttpath) + "timetrigger";
    client.subscribe(timeTriggerTopic.c_str(), 1);

    String firstPingTopic = String(configuration.mqttpath) + "FirstPing";
    client.publish(firstPingTopic.c_str(), "Hello ");

    String longlast = String(configuration.mqttpath) + "longlast";
    client.subscribe(longlast.c_str(), 1);

  } else {

    Serial.print("failed with state ");
    Serial.print(client.state());
  }
}
