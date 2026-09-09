#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "WiFiHandler.h"
#include "app_api.h"
#include "RelayTimer.h"
#include "BuzzerTimer.h"
#include "MQTTHandler.h"
#include "app.h"
#include "MQTTMessageHandler.h"


WiFiHandler wifiHandler;
RelayTimer relayTimer;
BuzzerTimer buzzerTimer;
MQTTHandler mqttHandler;
MQTTMessageHandler mqttMessageHandler;


void setup() {
  Serial.begin(115200);

  App::setup();
  AppApi::setup(App::getDeviceId());

  setupTimeRelay();
}


void setupTimeRelay() {
  wifiHandler.setupWiFi();

  relayTimer.setup();
  buzzerTimer.setup(D7);

  mqttHandler.setup(
    App::getDeviceId(),
    App::mqttHost,
    App::mqttPort
  );

  mqttHandler.setTopicActions(
    App::topicActions,
    sizeof(App::topicActions) / sizeof(App::topicActions[0])
  );

  mqttHandler.registerCallback(handleMQTTCallback);
  mqttHandler.registerDidFinishConnectCallback(
    handleMQTTDidFinishConnectCallback
  );

  mqttMessageHandler.setup(App::getDeviceId());
}


void loop() {
  wifiHandler.loopConnectWiFi();
  mqttHandler.loopConnectMQTT();

  buzzerTimer.loop();
  relayTimer.loop(handleRelayChange);

  delay(500);
}


/* --------------------------------------------------
 * Relay
 * -------------------------------------------------- */

void handleRelayChange(int index, uint8_t value) {
  String topic = App::getDeviceId() + "/switchon/relay";

  StaticJsonDocument<64> doc;
  doc["value"] = (bool)value;
  doc["index"] = index;

  String payload;
  serializeJson(doc, payload);

  mqttHandler.publish(
    topic.c_str(),
    payload.c_str(),
    false
  );
}


/* --------------------------------------------------
 * MQTT
 * -------------------------------------------------- */

void handleMQTTCallback(
  char* topic,
  byte* payload,
  unsigned int length
) {
  String deviceId = App::getDeviceId();

  mqttMessageHandler.handle(
    topic,
    payload,
    length,
    handleMQTTMessage
  );

  relayTimer.handleMQTTCallback(
    deviceId,
    topic,
    payload,
    length,
    handleRelayMQTTMessage
  );
}


void handleMQTTMessage(
  StaticJsonDocument<500> doc,
  char* topic,
  String message
) {
  String deviceId = App::getDeviceId();

  String refreshTopic = deviceId + "/refresh";
  String updateTopic = deviceId + "/update_version";
  String resetWifiTopic = deviceId + "/reset_wifi";

  if (strcmp(topic, refreshTopic.c_str()) == 0) {
    syncServerData();
    return;
  }

  if (strcmp(topic, updateTopic.c_str()) == 0) {
    AppApi::doUpdateOTA(App::getUpdateUrl());
    return;
  }

  if (strcmp(topic, resetWifiTopic.c_str()) == 0) {
    wifiHandler.resetWifi();
  }
}


void handleRelayMQTTMessage(
  StaticJsonDocument<500> doc,
  char* topic,
  String message
) {
  String deviceId = App::getDeviceId();

  String switchOnTopic = deviceId + "/switchon";

  if (strcmp(topic, switchOnTopic.c_str()) == 0) {

    if (doc.containsKey("longlast")) {
      buzzerTimer.activateFor(doc["longlast"]);
    }

    if (doc.containsKey("switch_value")) {
      buzzerTimer.setOn(doc["switch_value"]);
    }

    if (doc.containsKey("reminder")) {
      AppApi::addReminderMessage(message);
    }
  }

  if (strcmp(message.c_str(), "timeout") == 0) {
    String deviceInfo = AppApi::getDeviceInfo(deviceId);

    DynamicJsonDocument jsonDoc(500);

    if (deserializeJson(jsonDoc, deviceInfo)) {
      Serial.println("Failed to parse device info");
      return;
    }

    relayTimer.updateServerTime(
      jsonDoc["server_time"].as<String>()
    );
  }
}


/* --------------------------------------------------
 * MQTT Connected
 * -------------------------------------------------- */

void handleMQTTDidFinishConnectCallback() {
  syncServerData();

  String deviceId = App::getDeviceId();

  StaticJsonDocument<128> doc;
  doc["reset_reason"] = App::getResetReasonString();
  doc["device_id"] = deviceId;

  String payload;
  serializeJson(doc, payload);

  mqttHandler.publish(
    deviceId.c_str(),
    payload.c_str(),
    false
  );
}


/* --------------------------------------------------
 * Sync server
 * -------------------------------------------------- */

void syncServerData() {
  String deviceId = App::getDeviceId();

  String deviceInfo = AppApi::getDeviceInfo(deviceId);

  relayTimer.updateDeviceInfo(deviceInfo);

  String updateUrl = mqttMessageHandler.getUpdateUrl(deviceInfo);
  App::setUpdateUrl(updateUrl);

  AppApi::updateLastSeen(
    App::buildVersion,
    App::appVersion
  );
}
