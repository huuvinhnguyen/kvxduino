#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
#include <WiFiManager.h>
#include <WiFiHandler.h>
#include <Fingerprint.h>
#include <MQTTHandler.h>

Fingerprint fingerprint;
WiFiHandler wifiHandler;
MQTTHandler mqttHandler;

void setup() {
  Serial.begin(115200);
  wifiHandler.setupWiFi();
  mqttHandler.registerCallback(handleMQTTCallback);
  fingerprint.registerCallback(handleFingerprintCallback);
  fingerprint.setup();

}

void loop() {

  Serial.println("loop");

  if (WiFi.status() == WL_CONNECTED) {
    if (mqttHandler.connected()) {
      mqttHandler.loopConnectMQTT();
      Serial.println("main_loopConnectMQTT");


    } else {
      mqttHandler.loopReconnectMQTT();
      Serial.println("main_loopReconnectMQTT");
    }
  } else {
    Serial.println("main_loopConnectWiFi");

    wifiHandler.loopConnectWiFi();
  }

  fingerprint.loop();

  delay(1000);
}

void handleFingerprintCallback(char* message) {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["action"] = "enroll";
  jsonDoc["device_id"] = deviceId;
  jsonDoc["status"] = message;
  jsonDoc["enrollment_mode"] = fingerprint.enrollmentMode ;
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  String topic = "fingerprint";
  mqttHandler.publish(topic.c_str(), jsonString.c_str(), false);

}

void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {
  App::sendSlackMessage();

  payload[length] = '\0';

  // Khởi tạo một bộ đệm để chứa payload
  char buffer[length + 1];
  memcpy(buffer, payload, length + 1);

  // Khởi tạo một object JSON và parse payload
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, buffer);

  // Kiểm tra lỗi parse
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Truy cập các trường trong object JSON
  const char* message = doc["message"];
  Serial.print("Received message: ");
  Serial.print(message);


  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;
  Serial.print(str);


  String fingerprintTopic = deviceId + "/fingerprint";
  if (strcmp(topic, fingerprintTopic.c_str()) == 0) {
    String action = doc["action"];
    bool isActive = doc["active"];
    Serial.println("Action: fingerpirnt");

    if (action == "empty_database") {
      Serial.println("Action: empty_database detected, clearing fingerprint database...");
      fingerprint.deleteAllFingers();
      Serial.println("Fingerprint database cleared.");
    }

    if (action == "enroll" && isActive) { // isActive is one way (server to client)
      Serial.println("Action: enroll");
      if (doc["enrollment_mode"] == true) {
        int employeeId = doc["employee_id"];
        fingerprint.enroll(employeeId);
      } else {
//        fingerprint.cancelEnrollment();
      }

    }


    if (action == "delete_fingerprint") {
      int fingerId = doc["finger_id"];
      int employeeId = doc["employee_id"];
      String deviceFingerId = doc["device_finger_id"];

      fingerprint.deleteFingerprint(fingerId, [employeeId, deviceFingerId]() {
        App::deleteFingerprint(employeeId, deviceFingerId);
      });
    }
  }
}
