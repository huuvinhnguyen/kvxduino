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
#include <ElegantOTA.h>


ESP8266WebServer server(80);

const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;
RelayTimer relayTimer;
MQTTHandler mqttHandler;
MQTTMessageHandler mqttMessageHandler;
TimeClock timeClock;
Esp8266Server espServer;

uint8_t ledPin = 17;

void setup() {

  Serial.begin(115200);
  App::setup();
  AppApi::setup(App::getDeviceId());
  //  pinMode(pinPir2, INPUT);

  setupTimeRelay();
  server.begin();
  ElegantOTA.begin(&server);
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
  server.handleClient();
  ElegantOTA.loop();

  delay(1000);

}

void loopTimeRelay() {
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();
  relayTimer.loop([](String state, int index, uint8_t value) {
    AppApi::sendSlackMessage(state, index);
    Serial.println("switchRelayOnswitchRelayOn");
    int buildVersion = App::buildVersion;
    String appVersion = App::appVersion;
    AppApi::updateLastSeen(buildVersion, appVersion);
  });
}

void loopTimeClock() {
  espServer.loop();

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

  String deviceId = App::getDeviceId();
  mqttMessageHandler.handle(topic, payload, length, [mqttMessageHandler, deviceId](StaticJsonDocument<500> doc, char* topic, String message) {

    String deviceInfo = AppApi::getDeviceInfo(deviceId);
    relayTimer.updateDeviceInfo(deviceInfo);

    String refreshTopic = deviceId + "/refresh";
    if (strcmp(topic, refreshTopic.c_str()) == 0) {
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);

      int buildVersion = App::buildVersion;
      String appVersion = App::appVersion;
      AppApi::updateLastSeen(buildVersion, appVersion);

      String updateUrl = mqttMessageHandler.getUpdateUrl(deviceInfo);
      App::setUpdateUrl(updateUrl);
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

  relayTimer.handleMQTTCallback(deviceId, topic, payload, length, [relayTimer, deviceId](StaticJsonDocument<500> doc, char* topic, String message) {

    String pingTopic = deviceId + "/ping";
    if (strcmp(topic, pingTopic.c_str()) == 0) {
      //      App::sendDeviceMessage(message);
    }

    String switchOnTopic = deviceId + "/switchon";
    if (strcmp(topic, switchOnTopic.c_str()) == 0) {

      String action = doc["action"];
      if (action == "remove_reminder") {
        //        App::sendDeviceMessage(message);
      }

      if (doc.containsKey("longlast") ||
          doc.containsKey("switch_value") ||
          doc.containsKey("is_reminders_active")) {
        //        App::sendDeviceMessage(message);
        AppApi::sendSlackMessage();
      }

      if (doc.containsKey("reminder")) {
        AppApi::addReminderMessage(message);
      }
    }
  });
}

void handleMQTTDidFinishConnectCallback() {

  Serial.println("handleMQTTDidFinishCallback");
  String deviceId = App::getDeviceId();

  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  relayTimer.updateDeviceInfo(deviceInfo);

  String updateUrl = mqttMessageHandler.getUpdateUrl(deviceInfo);
  App::setUpdateUrl(updateUrl);

  int buildVersion = App::buildVersion;
  String appVersion = App::appVersion;
  AppApi::updateLastSeen(buildVersion, appVersion);

}
