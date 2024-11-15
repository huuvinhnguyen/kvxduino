#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiHandler.h"
#include "time.h"
#include "RelayTimer.h"
#include "MQTTHandler.h"
#include "App.h"

ESP8266WebServer server(80);

const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;
RelayTimer relayTimer;
MQTTHandler mqttHandler;

uint8_t ledPin = 17;


void setup() {

  Serial.begin(115200);
  pinMode(pinPir2, INPUT);

  wifiHandler.setupWiFi();
  relayTimer.setup();
  mqttHandler.registerCallback(handleMQTTCallback);

}

void loop() {

  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();
  server.handleClient();
  relayTimer.loop();
  delay(1000);

}

unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 5000; // 5 giây

void checkPir() {
  val = digitalRead(pinPir2);
  if (val == LOW) {
    Serial.println("No motion");
    delay(1000);
  } else {
    val = LOW;
    Serial.println("Motion detected  ALARM");

    // Kiểm tra thời gian để tránh gửi thông báo liên tục
    unsigned long currentTime = millis();
    if (currentTime - lastNotificationTime >= notificationInterval) {
      sendSlackMessage();
      lastNotificationTime = currentTime; // Cập nhật thời gian thông báo
    }

    delay(1000);
  }
}

void sendSlackMessage() {

  const char* webhookUrl = "http://103.9.77.155/devices/notify";

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient wifiClient;
    HTTPClient http;

    http.begin(wifiClient, webhookUrl);
    http.addHeader("Content-Type", "application/json");

    // Tạo đối tượng JSON
    time_t now = time(nullptr);
    DynamicJsonDocument jsonDoc(256);
    jsonDoc["id"] =  String(ESP.getChipId());
    jsonDoc["message"] = "Hello from esp8266";
    jsonDoc["time"] = now;
    jsonDoc["model"] = "esp8266";
    jsonDoc["sen"] = "pir";

    // Chuyển đổi đối tượng JSON thành chuỗi
    String payload;
    serializeJson(jsonDoc, payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error code: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  // Khởi tạo một bộ đệm để chứa payload
  char buffer[length + 1];
  memcpy(buffer, payload, length + 1);

  // Khởi tạo một object JSON và parse payload
  StaticJsonDocument<500> doc;
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

  String deviceId = mqttHandler.deviceId;

  String rootTopic = deviceId;
  if (strcmp(topic, rootTopic.c_str()) == 0) {
 
    String deviceInfo = App::getDeviceInfo(deviceId);
    relayTimer.updateRelays(deviceInfo);

  }

  String pingTopic = deviceId + "/ping";
  if (strcmp(topic, pingTopic.c_str()) == 0) {
    String messageString = relayTimer.getStateMessage(deviceId, "ping");
//    App::sendDeviceMessage(messageString);

  }

  String switchOnTopic = deviceId + "/switchon";
  if (strcmp(topic, switchOnTopic.c_str()) == 0) {

    int relayIndex = doc["relay_index"];
    String action = doc["action"];
    if (action == "remove_reminder") {
      String startTime = doc["start_time"];

      relayTimer.removeReminder(relayIndex, startTime, [relayTimer, deviceId]() {
        String messageString = relayTimer.getStateMessage(deviceId, "switchon");
        App::sendDeviceMessage(messageString);
      });

    }

    if (doc.containsKey("longlast")) {
      int longlast = doc["longlast"];
      relayTimer.setSwitchOnLast(relayIndex, longlast);
      String messageString = relayTimer.getStateMessage(deviceId, "switchon");
      App::sendDeviceMessage(messageString);
      App::sendSlackMessage();

    }

    if (doc.containsKey("switch_value")) {
      Serial.println("step 1: App::sendDeviceMessage(messageString)");

      bool isOn = doc["switch_value"];
      relayTimer.setOn(relayIndex, isOn);
      String messageString = relayTimer.getStateMessage(deviceId, "switchon");
      Serial.println("App::sendDeviceMessage(messageString)");
      Serial.println(messageString);

      App::sendSlackMessage();
      App::sendDeviceMessage(messageString);

    }

    if (doc.containsKey("reminder")) {
      String startTime = doc["reminder"]["start_time"];
      int duration = doc["reminder"]["duration"];
      String repeatType = doc["reminder"]["repeat_type"];
      relayTimer.addReminder(relayIndex, startTime, duration, repeatType);

      String messageString = relayTimer.getStateMessage(deviceId, "switchon");
      App::sendDeviceMessage(messageString);

    }
  }
}
