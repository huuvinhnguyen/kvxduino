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

#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ESP_DoubleResetDetector.h>

#include <ESPmDNS.h>


const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds

int countDevice = 0;

BLEConnector* connector = new BLEConnector();

WiFiClient net;
PubSubClient client(net);


void setup() {
  Serial.begin(115200);
  setupWiFi();
  connector->setupBLE();
  Serial.println("setup");
}

void loop() {
  Serial.println("loop");

//    if (WiFi.status() == WL_CONNECTED) {
//      if (client.connected()) {
//        client.loop();
//      } else {
//        loopConnectMQTT();
//      }
//    } else {
//      loopConnectWiFi();
//    }


  connector->loopConnectBLE();

  delay(1000);

}

void loopConnectWiFi() {

}

void loopConnectMQTT() {

}


void setupWiFi() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}
