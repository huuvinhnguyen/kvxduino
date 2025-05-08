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


ESP8266WebServer server(80);

const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;
RelayTimer relayTimer;
MQTTHandler mqttHandler;
TimeClock timeClock;
Esp8266Server espServer;

uint8_t ledPin = 17;


void setup() {

  Serial.begin(115200);
  //  pinMode(pinPir2, INPUT);

  wifiHandler.setupWiFi();
  relayTimer.setup();
  mqttHandler.setup(AppApi::getDeviceId());
  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(handleMQTTDidFinishConnectCallback);

}

void loop() {

  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();
  server.handleClient();
  relayTimer.loop([](String state, int index, uint8_t value) {
    AppApi::sendSlackMessage(state, index);
    String deviceId = mqttHandler.deviceId;
    AppApi::updateLastSeen();
    Serial.println("switchRelayOnswitchRelayOn");

  });

  delay(1000);

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

  relayTimer.handleMQTTCallback(mqttHandler.deviceId, topic, payload, length, [relayTimer](StaticJsonDocument<500> doc, char* topic, String message) {

    String deviceId = mqttHandler.deviceId;

    String refreshTopic = deviceId + "/refresh";
    if (strcmp(topic, refreshTopic.c_str()) == 0) {
      String deviceInfo = AppApi::getDeviceInfo(deviceId);
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);
      relayTimer.updateDeviceInfo(deviceInfo);
    }

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
  String deviceId = mqttHandler.deviceId;
  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  relayTimer.updateDeviceInfo(deviceInfo);
  AppApi::updateLastSeen();

}
