
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
#include <Ticker.h>


#define LED_BUILTIN 8

int countDevice = 0;

// Global objects
//BLEConnector connector;
MQTTHandler mqttHandler;
WiFiHandler wifiHandler;

TimeClock timeClock;
Esp32Server espServer;
RelayTimer relayTimer;
Pir pir;
MQTTMessageHandler mqttMessageHandler;
Ticker jobTicker;



void setup() {
  Serial.begin(115200);
  App::setup();
  AppApi::setup(App::getDeviceId());

  //  setupTimeClock();
  setupTimeRelay();
  //  server.begin();
  //  ElegantOTA.begin(&server);
  Serial.println("setup");
  //  jobTicker.attach(20, updateLastSeen);
  esp_log_level_set("*", ESP_LOG_VERBOSE);


}

void setupTimeRelay() {

  wifiHandler.setupWiFi();
  //  connector.setupBLE();
  //  connector.registerNotifyCallback(handleBLENotify);
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
  //  server.handleClient();
  //  ElegantOTA.loop();
  delay(100);

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


  mqttMessageHandler.handle(topic, payload, length, [mqttMessageHandler](StaticJsonDocument<500> doc, char* topic, String message) {

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
