
#include <ArduinoJson.h>
#include <BLEConnector.h>
#include <MQTTHandler.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>

#include <ESPmDNS.h>
#include <RelayTimer.h>
#include "time.h"
#include "app_api.h"
#include "esp32_pir.h"
#include "TimeClock.h"
#include "Esp32Server.h"
#include "app.h"

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

void setup() {
  Serial.begin(115200);
  App::setup();
  wifiHandler.setupWiFi();
  connector.setupBLE();
  connector.registerNotifyCallback(handleBLENotify);
  relayTimer.setup();
  mqttHandler.setup(AppApi::getDeviceId());
  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(handleMQTTDidFinishConnectCallback);

//  pir.setupPir();
//  pir.registerCallback(handlePirCallback);

  espServer.setup();
  espServer.registerCallback(handleServerSetTimeCallback);
  espServer.registerReminderCallback(handleServerSetReminderCallback);
  espServer.registerSwitchOnLonglastCallback(handleServerSwitchOnLonglastCallback);
  espServer.registerSwitchOnCallback(handleServerSwitchOnCallback);
  espServer.registerRemoveAllReminders(handleRemoveAllRemindersCallback);
  espServer.registerSetRemindersActive(handleSetRemindersActiveCallback);
  timeClock.setup();

  Serial.println("setup");
}

void loop() {

  Serial.println("loop");
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();


  connector.loopConnectBLE();
  relayTimer.loop([](String state, int index, uint8_t value) {
    AppApi::sendSlackMessage(state, index);
    String deviceId = mqttHandler.deviceId;
    AppApi::switchRelayOn(deviceId, index, value);
    AppApi::updateLastSeen();

  });

//  pir.loopPir();

  espServer.loop();

  timeClock.loop([](Time t) {
    espServer.timing(t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    espServer.updateDeviceInfo(timeClock.getStateMessage());
  });

  delay(1000);

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
  String deviceId = mqttHandler.deviceId;
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
  String deviceId = mqttHandler.deviceId;
  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  relayTimer.updateDeviceInfo(deviceInfo);
  AppApi::updateLastSeen();

}
