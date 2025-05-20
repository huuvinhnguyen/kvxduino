// MQTTMessageHandler.h
#pragma once

//#include <ArduinoJson.h>
//#include <functional>
//#include "RelayTimer.h"
//#include "MQTTHandler.h"
//
class MQTTMessageHandler {
  public:
    //  MQTTMessageHandler(RelayTimer& relayTimer, MQTTHandler& mqttHandler);
    //
    void setup(String deviceId);
    void updateServerTime(String serverTime);
    void handle(char* topic, byte* payload, unsigned int length, std::function<void(StaticJsonDocument<500>, char*, String)> callback);
    String getUpdateUrl(String deviceInfo);
    //
  private:
    String deviceId;
    //  RelayTimer& relayTimer;
    //  MQTTHandler& mqttHandler;
    //
    //  void handleRefresh(const String& deviceId);
    //  void handleSwitchOn(const String& message, const StaticJsonDocument<500>& doc);
};

void MQTTMessageHandler::setup(String deviceId) {
  this->deviceId = deviceId;
}

String MQTTMessageHandler::getUpdateUrl(String deviceInfo) {

  DynamicJsonDocument jsonDoc(500);
  DeserializationError error = deserializeJson(jsonDoc, deviceInfo);
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
  }

  String updateUrl = jsonDoc["device_info"]["update_url"].as<String>();

  return updateUrl;

}

void MQTTMessageHandler::handle(char* topic, byte* payload, unsigned int length, std::function<void(StaticJsonDocument<500>, char*, String)> callback) {
  payload[length] = '\0';

  // Khởi tạo một bộ đệm để chứa payload
  char buffer[length + 1];
  memcpy(buffer, payload, length + 1);

  // Khởi tạo một object JSON và parse payload
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, buffer);

  // Kiểm tra lỗi parse
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }


  String updateTopic = deviceId + "/update_version";
  if (strcmp(topic, updateTopic.c_str()) == 0) {
    Serial.println("update_version");
    callback(doc, topic, "");
  }

  String refreshTopic = deviceId + "/refresh";
  if (strcmp(topic, refreshTopic.c_str()) == 0) {
    callback(doc, topic, "");
  }

  String resetWifiTopic = deviceId + "/reset_wifi";
  if (strcmp(topic, resetWifiTopic.c_str()) == 0) {
    callback(doc, topic, "");
  }
}

void MQTTMessageHandler::updateServerTime(String serverTime) {
  // Parse thời gian dạng ISO 8601 (UTC): "2025-05-08T00:47:32"
  struct tm tm;
  if (!strptime(serverTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) {
    Serial.println("Failed to parse server time");
    return;
  }

  // Thiết lập tạm thời timezone là UTC để mktime trả đúng thời gian UTC
  setenv("TZ", "UTC0", 1);
  tzset();

  time_t utcTime = mktime(&tm); // mktime sẽ xử lý đúng vì timezone hiện tại là UTC

  // Cập nhật thời gian hệ thống ESP
  struct timeval now = { .tv_sec = utcTime };
  settimeofday(&now, nullptr);

  Serial.print("Updated system time to UTC: ");
  Serial.println(ctime(&utcTime));
}
