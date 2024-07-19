// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include "ViewInteractor.h"
// #include "DataDefault.h"
// #include <ESP8266mDNS.h>
// #include "Relay.h"
// #include "secrets.h"
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>
//#include <ESP_DoubleResetDetector.h>

#include <ESPmDNS.h>
//#include <NTPClient.h>
#include "Timer.h"
#include "Relay.h"


const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds

int countDevice = 0;

//BLEConnector* connector = new BLEConnector();
//
//MQTTHandler* mqttHandler = new MQTTHandler();
//
//
//WatchDog watchDog;
//WiFiHandler* wifiHandler = new WiFiHandler();
//Relay relay;
// Global objects
BLEConnector connector;
MQTTHandler mqttHandler;
WatchDog watchDog;
WiFiHandler wifiHandler;
Relay relay;


void setup() {
  Serial.begin(115200);
  wifiHandler.setupWiFi();
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
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

//  Serial.print("Notify callback for characteristic ");
//  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
//  Serial.print(": ");
//  Serial.println(str);

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
