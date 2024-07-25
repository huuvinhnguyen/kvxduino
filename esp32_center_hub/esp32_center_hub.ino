
#include <ArduinoJson.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include <WiFiHandler.h>

#include <ESPmDNS.h>
#include "Timer.h"
#include <Relay.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds

int countDevice = 0;


// Global objects
BLEConnector connector;
MQTTHandler mqttHandler;
WatchDog watchDog;
WiFiHandler wifiHandler;
Relay relay;

Ticker timeTicker;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  wifiHandler.setupWiFi();
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  mqttHandler.registerCallback(handleMQTTCallback);
//  timeTicker.attach(5, checkHeapMemory);
//  timeTicker.attach(0.5, checkCPUOverload);
  Serial.println("setup");
}

void loop() {
  Serial.println("loop");

  if (WiFi.status() == WL_CONNECTED) {
    if (mqttHandler.connected()) {
      mqttHandler.loopConnectMQTT();

    } else {
      mqttHandler.loopReconnectMQTT();
    }
  } else {
    wifiHandler.loopConnectWiFi();
  }

  connector.loopConnectBLE();

  delay(1000);

}

void handleBLENotify(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                     uint8_t* pData, size_t length, bool isNotify) {

  //  mqttHandler->publish("switchon", "okok");

  char str[length + 1];
  memcpy(str, pData, length);
  str[length] = '\0';

  // Parse the string to get temperature and humidity
  String dhtDataString = String(str);
  int commaIndex = dhtDataString.indexOf(',');
  if (commaIndex > 0) {
    String temperatureString = dhtDataString.substring(0, commaIndex);
    String humidityString = dhtDataString.substring(commaIndex + 1);

    float temperature = temperatureString.toFloat();
    float humidity = humidityString.toFloat();

    Serial.print("Parsed Temperature: ");
    Serial.print(temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Failed to parse dhtDataString");
  }

}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {
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

const uint32_t minHeapSize = 10000;
void checkHeapMemory() {
  uint32_t freeHeap = esp_get_free_heap_size();
  
  Serial.print("Free heap size: ");
  Serial.println(freeHeap);

  if (freeHeap < minHeapSize) {
    Serial.println("Heap memory overloaded, restarting...");
    ESP.restart();
  }
}

unsigned long previousMillis = 0;
const unsigned long maxExecutionTime = 30000;

void checkCPUOverload() {
  unsigned long currentMillis = millis();

  // Kiểm tra thời gian thực thi của vòng lặp chính
  if (currentMillis - previousMillis >= maxExecutionTime) {
    Serial.println("CPU overloaded, restarting...");
    ESP.restart();
  }
  
  // Cập nhật thời gian thực thi mới nhất
  previousMillis = currentMillis;
}
