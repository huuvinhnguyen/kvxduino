
#include <ArduinoJson.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>

#include <ESPmDNS.h>
#include <RelayTimer.h>
#include "time.h"
#include "App.h"



const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

int countDevice = 0;

// Global objects
BLEConnector connector;
MQTTHandler mqttHandler;
WiFiHandler wifiHandler;
RelayTimer relayTimer;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  wifiHandler.setupWiFi();
  relayTimer.setup();

  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  relayTimer.setup();
  mqttHandler.registerCallback(handleMQTTCallback);

  Serial.println("setup");
}

void loop() {

  Serial.println("loop");
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();


  connector.loopConnectBLE();
  relayTimer.loop([](String state, int index) {
    App::sendSlackMessage(state, index);
  });

  delay(1000);

}

void handleBLENotify(String jsonString) {


  // In chuỗi JSON để kiểm tra
  Serial.print("Received JSON: ");
  Serial.println(jsonString);

  // Tạo tài liệu JSON
  StaticJsonDocument<200> jsonDoc;

  // Phân tích chuỗi JSON
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  // Kiểm tra lỗi khi phân tích
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Trích xuất giá trị từ tài liệu JSON
  float temperature = jsonDoc["tem"];
  float humidity = jsonDoc["hum"];

  // In giá trị ra Serial
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  jsonDoc["hub_model"] = "esp32";
  String payload;
  serializeJson(jsonDoc, payload);
  String topic = "dht" ;
  mqttHandler.publish(topic.c_str(), jsonString.c_str(), false);

}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {

  relayTimer.handleMQTTCallback(mqttHandler.deviceId, topic, payload, length, [relayTimer](StaticJsonDocument<500> doc, char* topic, String message) {

    String deviceId = mqttHandler.deviceId;
    if (strcmp(topic, deviceId.c_str()) == 0) {
      String deviceInfo = App::getDeviceInfo(deviceId);
      relayTimer.updateRelays(deviceInfo);
    }

    String pingTopic = deviceId + "/ping";
    if (strcmp(topic, pingTopic.c_str()) == 0) {
      App::sendDeviceMessage(message);
    }

    String switchOnTopic = deviceId + "/switchon";
    if (strcmp(topic, switchOnTopic.c_str()) == 0) {

      String action = doc["action"];
      if (action == "remove_reminder") {
        App::sendDeviceMessage(message);
      }

      if (doc.containsKey("longlast") || 
      doc.containsKey("switch_value") || 
      doc.containsKey("is_reminders_active")) {
        App::sendDeviceMessage(message);
        App::sendSlackMessage();
      }

      if (doc.containsKey("reminder")) {
        App::addReminderMessage(message);
      }
    }
  }); 
}
