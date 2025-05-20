
#include <ArduinoJson.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <RelayTimer.h>
#include "time.h"
#include "app_api.h"
#include "esp32_pir.h"
#include "TimeClock.h"
#include "Esp32Server.h"
#include "app.h"
#include "MQTTMessageHandler.h"
#include <ElegantOTA.h>


WebServer server(80);

#define LED_BUILTIN 8

int countDevice = 0;

// Global objects
BLEConnector connector;
MQTTHandler mqttHandler;
WiFiHandler wifiHandler;

TimeClock timeClock;
Esp32Server espServer;
RelayTimer relayTimer;
Pir pir;
MQTTMessageHandler mqttMessageHandler;


void setup() {
  Serial.begin(115200);
  App::setup();
  AppApi::setup(App::getDeviceId());

  //  setupTimeClock();
  setupTimeRelay();
  server.begin();
  ElegantOTA.begin(&server);
  Serial.println("setup");
}

void setupTimeRelay() {

  wifiHandler.setupWiFi();
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  relayTimer.setup();
  mqttHandler.setup(App::getDeviceId(), App::mqttHost, App::mqttPort);
  mqttHandler.setTopicActions(App::topicActions, sizeof(App::topicActions) / sizeof(App::topicActions[0]));
  mqttMessageHandler.setup(App::getDeviceId());

  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(handleMQTTDidFinishConnectCallback);
  mqttMessageHandler.setup(App::getDeviceId());

  //  pir.setupPir();
  //  pir.registerCallback(handlePirCallback);

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

  Serial.println("loop");
  loopTimeRelay();
  server.handleClient();
  ElegantOTA.loop();
  delay(1000);

}

void loopTimeRelay() {
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();
  connector.loopConnectBLE();
  relayTimer.loop([](String state, int index, uint8_t value) {
    AppApi::sendSlackMessage(state, index);
    int buildVersion = App::buildVersion;
    String appVersion = App::appVersion;
    AppApi::updateLastSeen(buildVersion, appVersion);

  });
  //  pir.loopPir();
}

void loopTimeClock() {
  espServer.loop();

  timeClock.loop([](Time t) {
    espServer.timing(t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    espServer.updateDeviceInfo(timeClock.getStateMessage());
  });
}


void handleBLENotify(String jsonString) {

  // In chuỗi JSON để kiểm tra
  Serial.print("Received JSON: ");
  Serial.println(jsonString);

  // Tạo tài liệu JSON
  StaticJsonDocument<200> jsonDoc;

  // Phân tích chuỗi JSON
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  // Kiểm tra lỗi khi phân tích
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Trích xuất giá trị từ tài liệu JSON
  float temperature = jsonDoc["tem"];
  float humidity = jsonDoc["hum"];

  // In giá trị ra Serial
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  jsonDoc["hub_model"] = "esp32";
  String payload;
  serializeJson(jsonDoc, payload);
  String topic = "dht" ;
  mqttHandler.publish(topic.c_str(), jsonString.c_str(), false);

}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {

  String deviceId = App::getDeviceId();
  mqttMessageHandler.handle(topic, payload, length, [mqttMessageHandler, deviceId](StaticJsonDocument<500> doc, char* topic, String message) {

    String deviceId = App::getDeviceId();

    String deviceInfo = AppApi::getDeviceInfo(deviceId);
    relayTimer.updateDeviceInfo(deviceInfo);


    String refreshTopic = deviceId + "/refresh";
    if (strcmp(topic, refreshTopic.c_str()) == 0) {
//      String deviceInfo = AppApi::getDeviceInfo(deviceId);
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);

      int buildVersion = App::buildVersion;
      String appVersion = App::appVersion;
      AppApi::updateLastSeen(buildVersion, appVersion);

//      relayTimer.updateDeviceInfo(deviceInfo);

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

    String refreshTopic = deviceId + "/refresh";
    if (strcmp(topic, refreshTopic.c_str()) == 0) {
      String deviceInfo = AppApi::getDeviceInfo(deviceId);
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);
      relayTimer.updateDeviceInfo(deviceInfo);

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

      if (doc.containsKey("longlast") ||
          doc.containsKey("switch_value") ||
          doc.containsKey("is_reminders_active")) {
        //        AppApi::sendDeviceMessage(message);
        AppApi::sendSlackMessage();
      }

      if (doc.containsKey("reminder")) {
        AppApi::addReminderMessage(message);
      }
    }
  });
}

void handlePirCallback() {
  Serial.println("Object Detected 222");
  String deviceId = App::getDeviceId();
  //    AppApi::sendSlackMessage();
  AppApi::sendTrigger(deviceId);

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
