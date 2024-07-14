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
#include <ESP_DoubleResetDetector.h>

#include <ESPmDNS.h>
//#include <NTPClient.h>
#include "Timer.h"
#include "Relay.h"


const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds

int countDevice = 0;

BLEConnector* connector = new BLEConnector();
MQTTHandler* mqttHandler = new MQTTHandler();


WatchDog watchDog;
WiFiHandler* wifiHandler = new WiFiHandler();
Relay relay;


void setup() {
  Serial.begin(115200);
  wifiHandler->setupWiFi();
  connector->setupBLE();
  connector->registerNotifyCallback(handleBLENotify);
  Serial.println("setup");
}

void loop() {
  Serial.println("loop");

  if (WiFi.status() == WL_CONNECTED) {
    if (mqttHandler->connected()) {
      mqttHandler->loopConnectMQTT();
      
    } else {
      mqttHandler->loopReconnectMQTT();
    }
  } else {
    wifiHandler->loopConnectWiFi();
  }

  connector->loopConnectBLE();

  delay(1000);

}

void handleBLENotify(int32_t notifyValue) {
  Serial.print("Handled notify value: ");
  Serial.println(notifyValue);
  mqttHandler->publish("switchon", "okok");
}
