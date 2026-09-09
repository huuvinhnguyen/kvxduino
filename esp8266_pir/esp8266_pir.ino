#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiHandler.h"
#include "time.h"
#include "app_api.h"
#include "MQTTHandler.h"
#include "app.h"
#include "Esp8266Server.h"
#include "MQTTMessageHandler.h"
#include <Ticker.h>


bool previousPirState = LOW;
const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;

MQTTHandler mqttHandler;
MQTTMessageHandler mqttMessageHandler;

uint8_t ledPin = 17;

void setup() {

  Serial.begin(115200);
  App::setup();
  AppApi::setup(App::getDeviceId());
    pinMode(pinPir2, INPUT_PULLUP);

  setupTimeRelay();


}

void setupTimeRelay() {
  wifiHandler.setupWiFi();
  mqttHandler.setup(App::getDeviceId(), App::mqttHost, App::mqttPort);
  mqttHandler.setTopicActions(App::topicActions, sizeof(App::topicActions) / sizeof(App::topicActions[0]));
  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(handleMQTTDidFinishConnectCallback);
  mqttMessageHandler.setup(App::getDeviceId());
}



void loop() {

   bool pirState = digitalRead(pinPir2) == HIGH;
  if (pirState && !previousPirState) {
    Serial.println("PIR detected, sending trigger");
    AppApi::sendTrigger(App::getDeviceId());
  }
  previousPirState = pirState;
  delay(500);
//  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());

}

void loopTimeRelay() {
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();  
 
}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {


  mqttMessageHandler.handle(topic, payload, length, [](StaticJsonDocument<500> doc, char* topic, String message) {

    String deviceId = App::getDeviceId();
    String refreshTopic = deviceId + "/refresh";
    if (strcmp(topic, refreshTopic.c_str()) == 0) {
      syncServerData();
    }

    String updateTopic = deviceId + "/update_version";
    if (strcmp(topic, updateTopic.c_str()) == 0) {
      String updateUrl = App::getUpdateUrl();
      Serial.println("updateUrl: ");
      Serial.print(updateUrl);
      AppApi::doUpdateOTA(updateUrl);
    }

    String resetWifiTopic = deviceId + "/reset_wifi";
    if (strcmp(topic, resetWifiTopic.c_str()) == 0) {
      Serial.println("resetting wifi");
      wifiHandler.resetWifi();
    }

  });

  
}

void handleMQTTDidFinishConnectCallback() {

  Serial.println("handleMQTTDidFinishCallback");
  syncServerData();


  String resetReason = App::getResetReasonString();
  String deviceId = App::getDeviceId();

  StaticJsonDocument<128> doc;
  doc["reset_reason"] = resetReason;
  doc["device_id"] = deviceId;

  //Chuyển thành chuỗi JSON
  String payload;
  serializeJson(doc, payload);
  mqttHandler.publish(deviceId.c_str(), payload.c_str(), false);
}

void syncServerData() {
  String deviceId = App::getDeviceId();
  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  delay(100);

  String updateUrl = mqttMessageHandler.getUpdateUrl(deviceInfo);
  App::setUpdateUrl(updateUrl);

  int buildVersion = App::buildVersion;
  String appVersion = App::appVersion;
  AppApi::updateLastSeen(buildVersion, appVersion);

}
