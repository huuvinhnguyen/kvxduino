
#include <ArduinoJson.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>

#include <ESPmDNS.h>
#include "Timer.h"
#include "Relay.h"

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

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  wifiHandler.setupWiFi();
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  mqttHandler.registerCallback(handleMQTTCallback);

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

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;
  Serial.print(str);
  //
  //  String publishTopic = String(configuration.mqttpath) + "switch";
  //
  //  relay.handleMessage(topic, str);
  //
  //
  //  String openValueTopic = String(configuration.mqttpath) + "openvalue";
  //  if (strcmp(topic, openValueTopic.c_str()) == 0) {
  //
  //    if (str.equals("done")) {
  //      Serial.println("#done");
  //    } else {
  //
  //      ser_pos_fishtank = str.toInt();
  //      Serial.print("set value open: ");
  //      Serial.println(ser_pos_fishtank);
  //      //      client.publish(topic, "done", false);
  //    }
  //  }
  //
  //  String timeTriggerTopic = String(configuration.mqttpath) + "timetrigger";
  //  if (strcmp(topic, timeTriggerTopic.c_str()) == 0) {
  //
  //    if (str.equals("done")) {
  //      Serial.println("#done");
  //    } else {
  //
  //      watchDog.setTimeString(str);
  //      Serial.print("trigger string: ");
  //      Serial.println(str);
  //      //      client.publish(topic, "done", false);
  //    }
  //  }
  //
  Serial.println("ok");
  //  Serial.println("-----------------------");
  //  //  digitalWrite(ledPin, !digitalRead(ledPin));

}
