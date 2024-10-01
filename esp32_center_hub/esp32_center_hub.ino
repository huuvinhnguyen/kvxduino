
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
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  relayTimer.setup();
  mqttHandler.registerCallback(handleMQTTCallback);

  Serial.println("setup");
}

void loop() {
  
  Serial.println("loop");

  if (WiFi.status() == WL_CONNECTED) {
    if (mqttHandler.connected()) {
      mqttHandler.loopConnectMQTT();
      Serial.println("loopConnectMQTT");


    } else {
      mqttHandler.loopReconnectMQTT();
      Serial.println("loopReconnectMQTT");
    }
  } else {
    Serial.println("loopConnectWiFi");

    wifiHandler.loopConnectWiFi();
  }

  connector.loopConnectBLE();
  relayTimer.loopTriggerRelay();

  delay(1000);

}

//void handleBLENotify(BLERemoteCharacteristic* pBLERemoteCharacteristic,
//                     uint8_t* pData, size_t length, bool isNotify) {
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
  App::sendSlackMessage();

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
    relayTimer.relay.handleMessage("switch", String(value));
  }

  String timeTriggerTopic = deviceId + "/timetrigger";
  if (strcmp(topic, timeTriggerTopic.c_str()) == 0) {
    String value = doc["value"];
    relayTimer.watchDog.setTimeString(value);
    String jsonString = relayTimer.getStateMessage(deviceId);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);
  }

  String longlastTopic = deviceId + "/longlast";
  if (strcmp(topic, longlastTopic.c_str()) == 0) {
    int value = doc["value"];
    relayTimer.relay.longlast = value;
    String jsonString = relayTimer.getStateMessage(deviceId);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);
  }

  String switchOnTopic = deviceId + "/switchon";
  if (strcmp(topic, switchOnTopic.c_str()) == 0) {
    int longlast = doc["longlast"];
    relayTimer.relay.longlast = longlast;
    relayTimer.relay.switchOn();
    String jsonString = relayTimer.getStateMessage(deviceId);
    client.publish(deviceId.c_str(), jsonString.c_str(), true);
  }

}
