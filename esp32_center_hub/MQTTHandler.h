#include <PubSubClient.h>
#include <WiFiClient.h>

//#include "app.h"

//using MQTTCallback = void(*)(char*, byte*, unsigned int);
using MQTTCallback = void(*)(char* topic, byte* payload, unsigned int length);
using MQTTDidFinishConnectCallback = void(*)(void);


class MQTTHandler {

  private:
    WiFiClient net;
    PubSubClient client;
    String deviceId = "";
    String mqttHost = "";
    int port;
    const char* const* topicActions = nullptr;
    int topicActionsCount = 0;
    static MQTTCallback callbackFunc;
    static MQTTDidFinishConnectCallback didFinishConnectCallbackFunc;
    String generateRandomUUID() {
      // Generate random parts for the UUID (32-bit chunks)
      uint32_t part1 = random(0xFFFFFFFF);  // First 32 bits
      uint32_t part2 = random(0xFFFFFFFF);  // Second 32 bits
      uint32_t part3 = random(0xFFFFFFFF);  // Third 32 bits
      uint32_t part4 = random(0xFFFFFFFF);  // Fourth 32 bits

      // Combine parts into a UUID string (8-4-4-4-12 format)
      char uuid[37];  // UUID is 36 characters + null terminator
      snprintf(uuid, sizeof(uuid), "%08X-%04X-%04X-%04X-%012X",
               part1,  // First 8 characters
               (uint16_t)(part2 >> 16), (uint16_t)(part2 & 0xFFFF), // Second 4 and Third 4 characters
               (uint16_t)(part3 >> 16), (uint32_t)(part4));  // Fourth 4 characters and remaining 12 characters

      return String(uuid);
    }

  public:

    MQTTHandler() : net(), client(net) {} // Khởi tạo PubSubClient với WiFiClient
    long lastReconnectMQTTAttempt = 0;

    void setup(String deviceId, String mqttHost, int port) {
      this->mqttHost = mqttHost;
      this->port = port;
      this->deviceId = deviceId;

    }

    void registerCallback(MQTTCallback callback) {
      callbackFunc = callback;
    }

    void registerDidFinishConnectCallback(MQTTDidFinishConnectCallback callback) {
      didFinishConnectCallbackFunc = callback;
    }


    bool connected() {
      if (client.connected() ) {
        return true;
      } else {
        return false;
      }
    }

    void loopConnectMQTT() {

      client.loop();

      if (client.connected()) {
        Serial.println("loopConnectMQTT");

      } else {
        loopReconnectMQTT();
        Serial.println("loopReconnectMQTT");
      }

    }

    bool isFirstConnection = true;
    void loopReconnectMQTT() {

      Serial.println("reconnecting Mqtt");

      long now = millis();
      if (isFirstConnection || (now - lastReconnectMQTTAttempt > 15000)) {
        isFirstConnection = false;
        Serial.println("inside reconnecting Mqtt");


        lastReconnectMQTTAttempt = now;
        // Attempt to connect
        delay(1000);
        connectMQTT();

        if (client.connected()) {
          lastReconnectMQTTAttempt = 0;
        }
      }
    }

    uint32_t getChipId() {
#if defined(ESP32)
      uint32_t chipId = 0;
      for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }
      return chipId;
#elif defined(ESP8266)
      return ESP.getChipId();
#else
#error "Unsupported platform. This code supports only ESP32 and ESP8266."
#endif
    }

    void connectMQTT() {

      if (client.connected()) {
        return;
      }


      //      net.setTrustAnchors(&cert);
      //      net.setClientRSACert(&client_crt, &key);

      client.setServer(mqttHost.c_str(), port);
      client.setCallback(callback);

      Serial.println(mqttHost);
      Serial.println(deviceId);

      Serial.println("Connecting to MQTT...");

      //      uint32_t chipId = getChipId();
      String clientId = generateRandomUUID();


      if (client.connect(clientId.c_str())) {

        Serial.println("connected");

        String jsonString = getInitialMessage(deviceId);
        client.publish(deviceId.c_str(), jsonString.c_str(), false);

        for (int i = 0; i < topicActionsCount; ++i) {
          String topic = deviceId + "/" + topicActions[i];
          Serial.println("topic: " + topic);
          client.publish(topic.c_str(), "", true); // Xóa retained
          delay(200); // Chờ chút để broker xử lý
          client.subscribe(topic.c_str(), 1);
        }

        didFinishConnectCallbackFunc();

      } else {

        Serial.print("failed with state ");
        Serial.print(client.state());
      }
    }

    void setTopicActions(const char* const* actions, int count) {
      this->topicActions = actions;
      this->topicActionsCount = count;
    }

    void publish(const char* topic, const char* message, bool isRetained) {
      //      String switchonTopic = deviceId + "/" + topic;
      if (client.connected()) {
        client.publish(topic, message, isRetained);
        Serial.print("Message published to topic: ");
        Serial.print(topic);
        Serial.print(" Message: ");
        Serial.println(message);
      } else {
        Serial.println("Client not connected, message not published");
      }
    }

    String getInitialMessage(String deviceId) {
      time_t now = time(nullptr);  // Get the current epoch time
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["device_type"] = "initial";
      jsonDoc["device_id"] = deviceId;
      jsonDoc["update_at"] = now;
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      return jsonString;
    }

    static void callback(char* topic, byte* payload, unsigned int length) {
      callbackFunc(topic, payload, length);

    }

};
MQTTCallback MQTTHandler::callbackFunc = nullptr;
MQTTDidFinishConnectCallback MQTTHandler::didFinishConnectCallbackFunc = nullptr;
