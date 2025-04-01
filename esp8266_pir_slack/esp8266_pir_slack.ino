#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiHandler.h"
#include "time.h"
#include "RelayTimer.h"
#include "MQTTHandler.h"
#include "App.h"
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

  //  wifiHandler.setupWiFi();
  relayTimer.setup();
  //  mqttHandler.setup(App::getDeviceId());
  //    mqttHandler.registerCallback(handleMQTTCallback);

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

  //  wifiHandler.loopConnectWiFi();
  //  mqttHandler.loopConnectMQTT();
  //  server.handleClient();
  //  relayTimer.loop([](String state, int index) {
  //    App::sendSlackMessage(state, index);
  //  });
  espServer.loop();

  timeClock.loop([](Time t) {
    espServer.timing(t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    espServer.updateDeviceInfo(timeClock.getStateMessage());
  });

  delay(1000);

}

unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 5000; // 5 giây

void checkPir() {
  val = digitalRead(pinPir2);
  if (val == LOW) {
    Serial.println("No motion");
    delay(1000);
  } else {
    val = LOW;
    Serial.println("Motion detected  ALARM");

    // Kiểm tra thời gian để tránh gửi thông báo liên tục
    unsigned long currentTime = millis();
    if (currentTime - lastNotificationTime >= notificationInterval) {
      String deviceId = mqttHandler.deviceId;
      App::sendPirSlackMessage(deviceId);
      lastNotificationTime = currentTime; // Cập nhật thời gian thông báo
    }

    delay(1000);
  }
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
    if (strcmp(topic, deviceId.c_str()) == 0) {
      String deviceInfo = App::getDeviceInfo(deviceId);
      relayTimer.updateRelays(deviceInfo);
    }

    String pingTopic = deviceId + "/ping";
    if (strcmp(topic, pingTopic.c_str()) == 0) {
      App::sendDeviceMessage(message);
    }

    String switchOnTopic = deviceId + "/switchon";
    if (strcmp(topic, switchOnTopic.c_str()) == 0) {

      String action = doc["action"];
      if (action == "remove_reminder") {
        App::sendDeviceMessage(message);
      }

      if (doc.containsKey("longlast") ||
          doc.containsKey("switch_value") ||
          doc.containsKey("is_reminders_active")) {
        App::sendDeviceMessage(message);
        App::sendSlackMessage();
      }

      if (doc.containsKey("reminder")) {
        App::addReminderMessage(message);
      }
    }
  });
}
