#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiHandler.h"
#include "time.h"
#include "app_api.h"
#include "RelayTimer.h"
#include "MQTTHandler.h"
#include "app.h"
#include "TimeClock.h"
#include "Esp8266Server.h"
#include "MQTTMessageHandler.h"
#include <Ticker.h>


ESP8266WebServer server(80);

const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;
RelayTimer relayTimer;
MQTTHandler mqttHandler;
MQTTMessageHandler mqttMessageHandler;
TimeClock timeClock;
Esp8266Server espServer;
Ticker jobTicker;

uint8_t ledPin = 17;

void setup() {

  Serial.begin(115200);
  App::setup();
  AppApi::setup(App::getDeviceId());
  //  pinMode(pinPir2, INPUT);

  setupTimeRelay();
  

}

void setupTimeRelay() {
  wifiHandler.setupWiFi();
  relayTimer.setup();
  mqttHandler.setup(App::getDeviceId(), App::mqttHost, App::mqttPort);
  mqttHandler.setTopicActions(App::topicActions, sizeof(App::topicActions) / sizeof(App::topicActions[0]));
  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(handleMQTTDidFinishConnectCallback);
  mqttMessageHandler.setup(App::getDeviceId());
}

void setupTimeClock() {
  espServer.setup();
  espServer.registerCallback(handleServerSetTimeCallback);
  espServer.registerReminderCallback(handleServerSetReminderCallback);
  espServer.registerSwitchOnLonglastCallback(handleServerSwitchOnLonglastCallback);
  espServer.registerSwitchOnCallback(handleServerSwitchOnCallback);
  espServer.registerRemoveAllReminders(handleRemoveAllRemindersCallback);
  espServer.registerSetRemindersActive(handleSetRemindersActiveCallback);
  timeClock.setup();
}

void loop() {
  loopTimeRelay();
  delay(500);
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());

}

void loopTimeRelay() {
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();
  //  connector.loopConnectBLE();
  relayTimer.loop([mqttHandler](int index, uint8_t value) {
    Serial.println("relayTimer.loop state, index");

    String deviceId = App::getDeviceId();
    String switchOnTopic = deviceId + "/switchon/relay";

    // 1. Tạo JSON document
    StaticJsonDocument<128> doc;

    // 2. Gán giá trị
    doc["value"] = (bool)value;
    doc["index"] = index;

    // 3. Chuyển thành chuỗi JSON
    String payload;
    serializeJson(doc, payload);

    // 4. Gửi MQTT
    mqttHandler.publish(switchOnTopic.c_str(), payload.c_str(), false);
  });
  //  pir.loopPir();
}

void loopTimeClock() {

  timeClock.loop([](Time t) {
    espServer.timing(t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    espServer.updateDeviceInfo(timeClock.getStateMessage());
  });
}

void handleServerSetTimeCallback(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  timeClock.setTime(year, month, day, hour, minute, second);
}

void handleServerSetReminderCallback(int relayIndex, String startTime, int duration, String repeatType) {
  timeClock.addReminder(relayIndex, startTime, duration, repeatType);
  timeClock.saveReminderData();
}

void handleServerSwitchOnCallback(int relayIndex, bool isOn) {
  timeClock.setOn(relayIndex, isOn);
}

void handleServerSwitchOnLonglastCallback(int relayIndex, int longlast) {
  timeClock.setSwitchOnLast(relayIndex, longlast);
}

void handleRemoveAllRemindersCallback() {
  timeClock.removeAllReminders();
}

void handleSetRemindersActiveCallback(int relayIndex, bool isActive) {
  timeClock.setRemindersActive(relayIndex, isActive);
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

  String deviceId = App::getDeviceId();

  relayTimer.handleMQTTCallback(deviceId, topic, payload, length, [relayTimer, deviceId](StaticJsonDocument<500> doc, char* topic, String message) {


    if (strcmp("timeout", message.c_str()) == 0) {
      String deviceInfo = AppApi::getDeviceInfo(deviceId);
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);

      DynamicJsonDocument jsonDoc(500);
      DeserializationError error = deserializeJson(jsonDoc, deviceInfo);
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
      }

      relayTimer.updateServerTime(jsonDoc["server_time"].as<String>());

    }

    String pingTopic = deviceId + "/ping";
    if (strcmp(topic, pingTopic.c_str()) == 0) {
      //      AppApi::sendDeviceMessage(message);
    }

    String switchOnTopic = deviceId + "/switchon";
    if (strcmp(topic, switchOnTopic.c_str()) == 0) {

      String action = doc["action"];
      if (action == "remove_reminder") {
        //        AppApi::sendDeviceMessage(message);
      }

      if (doc.containsKey("reminder")) {
        AppApi::addReminderMessage(message);
      }
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

  //Chuyển thành chuỗi JSON
  String payload;
  serializeJson(doc, payload);
  mqttHandler.publish(deviceId.c_str(), payload.c_str(), false);
}

void syncServerData() {
  String deviceId = App::getDeviceId();
  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  relayTimer.updateDeviceInfo(deviceInfo);
  delay(100);

  String updateUrl = mqttMessageHandler.getUpdateUrl(deviceInfo);
  App::setUpdateUrl(updateUrl);

  int buildVersion = App::buildVersion;
  String appVersion = App::appVersion;
  AppApi::updateLastSeen(buildVersion, appVersion);

}
