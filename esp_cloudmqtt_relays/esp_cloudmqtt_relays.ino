#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include "Relay.h"
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>


WebServer server(80);
uint8_t relayPin = 13;
uint8_t relayPins[6] = {5, 4, 0, 2, 15, 16};
Relay relay;


struct Configuration {

  char mqttServer[60] = "103.9.77.155\0";
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char mqttpath[30];
  char wifiSSID[30] = "Huu Si\0";
  char wifiPassword[30] = "hai1989@\0";

} configuration;

//WiFiClientSecure net;
WiFiClient net;

//BearSSL::X509List cert(cacert);
//BearSSL::X509List client_crt(client_cert);
//BearSSL::PrivateKey key(privkey);
PubSubClient client(net);

uint8_t ledPin = 17;


void setup() {

  Serial.begin(115200);
  setupTimeClient();

  loadDataDefault();

  //  configureServer();

  setupWiFi();

  //  relay.setup("");
//  digitalWrite(D4, LOW);



}


void loop() {

//  MDNS.update();

  if (WiFi.status() == WL_CONNECTED) {

    if (client.connected()) {

      client.loop();
    } else {

      loopConnectMQTT();
    }
  } else {

    loopConnectWifi();
  }

  //  server.handleClient();

  updateTriggerRelay();
  relay.loop([](int count) {
    //      String switchonTopic = String(configuration.mqttpath) + "switchon";
    //      Serial.println(switchonTopic);
    //
    //      client.publish(switchonTopic.c_str(), "done", true);
    //
    //      String switchTopic = String(configuration.mqttpath) + "switch";
    //      Serial.println(switchTopic);
    //
    //      client.publish(switchTopic.c_str(), "0", true);
    //
    //      Serial.println(count);

  });
}

#include <NTPClient.h>
#include "Timer.h"
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
WatchDog watchDog;


time_t now;
time_t nowish = 1510592825;
void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}



void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  // Khởi tạo một bộ đệm để chứa payload
  char buffer[length + 1];
  memcpy(buffer, payload, length + 1);

  // Khởi tạo một object JSON và parse payload
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, buffer);

  // Kiểm tra lỗi parse
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Truy cập các trường trong object JSON
  const char* message = doc["message"];
  Serial.print("Received message: ");
  Serial.print(message);


  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;
  Serial.print(str);

  if (strcmp(topic + strlen(topic) - 6, "switch") == 0) {
    int value = doc["value"];
    relay.handleMessage("switch", String(value));
    String jsonString = getStateMessage(relay);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);
  }

  String timeTriggerTopic = deviceId + "/timetrigger";
  if (strcmp(topic, timeTriggerTopic.c_str()) == 0) {
    String value = doc["value"];
    watchDog.setTimeString(value);
    String jsonString = getStateMessage(relay);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);

  }

  String longlastTopic = deviceId + "/longlast";
  if (strcmp(topic, longlastTopic.c_str()) == 0) {
    int value = doc["value"];
    relay.longlast = value;
    String jsonString = getStateMessage(relay);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);

  }

  String switchOnTopic = deviceId + "/switchon";
  if (strcmp(topic, switchOnTopic.c_str()) == 0) {
    int longlast = doc["longlast"];
    relay.longlast = longlast;
    relay.switchOn();
    String jsonString = getStateMessage(relay);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);

  }

}

void setupTimeClient() {
  timeClient.begin();
}

void updateTriggerRelay() {
  timeClient.update();

  bool isActive = watchDog.isAlarmAtTime(timeClient.getHours(), timeClient.getMinutes());
  if (isActive) {
    Serial.println("Activate relay");
    relay.switchOn();
  }
}


void setupWiFi() {
  delay(500);
//  String host = String(ESP.getChipId());
  //  const char* host = "thietbi";
  String host = "thietbi";
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  delay(500);

  //  const char* ssid = "khuonvienxanh";
//  String ssid = "kv_" + String(ESP.getChipId());
  String ssid = "kv_";


  WiFi.softAP(ssid.c_str());
  IPAddress myIP = WiFi.softAPIP();

  long now = millis();

  while ((millis() - now) < 5000) {

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

    if (strcmp(serverString.c_str(), "") != 0) {
      strcpy(configuration.mqttServer, serverString.c_str());
    }
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
  //  WiFi.begin(configuration.wifiSSID, configuration.wifiPassword);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.println("Password: ");
  Serial.println(WIFI_PASSWORD);


  //  Serial.println(configuration.wifiPassword);

  Serial.println("wifi connecting...");
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
  if (client.connected()) {
    return;
  }

  NTPConnect();

//  net.setTrustAnchors(&cert);
//  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, 1883);
  client.setCallback(callback);

  Serial.println(MQTT_HOST);
  Serial.println(deviceId);

  Serial.println("Connecting to MQTT...");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  if (client.connect(deviceId.c_str())) {

    Serial.println("connected");
    String switchTopic = deviceId + "/switch";
    client.subscribe(switchTopic.c_str(), 1);
    //    StaticJsonDocument<200> jsonDoc;
    //    jsonDoc["value"] = relay.value;
    //    String jsonString;
    //    serializeJson(jsonDoc, jsonString);
    //    client.publish(deviceId.c_str(), jsonString.c_str(), true);
    //

    relay.setup("");
    String switchonTopic = deviceId + "/switchon";
    client.subscribe(switchonTopic.c_str(), 1);

    String timeTriggerTopic = deviceId + "/timetrigger";
    client.subscribe(timeTriggerTopic.c_str(), 1);

    //    String jsonString = getStateMessage(relay);
    //    client.publish(deviceId.c_str(), jsonString.c_str(), true);
    client.subscribe(deviceId.c_str(), 1);

    String longlast = deviceId + "/longlast";
    client.subscribe(longlast.c_str(), 1);

  } else {

    Serial.print("failed with state ");
    Serial.print(client.state());
  }
}

String getStateMessage(Relay relay) {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["device_type"] = "switch";
  jsonDoc["device_id"] = deviceId;
  jsonDoc["value"] = relay.value;
  jsonDoc["update_at"] = timeClient.getEpochTime();
  jsonDoc["longlast"] = relay.longlast;
  jsonDoc["timetrigger"] = watchDog.getTimeString();
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  return jsonString;
}
