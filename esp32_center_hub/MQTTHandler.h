#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include "app.h"

//using MQTTCallback = void(*)(char*, byte*, unsigned int);
using MQTTCallback = void(*)(char* topic, byte* payload, unsigned int length);


struct Configuration {

  char mqttServer[60] = "103.9.77.155\0";
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char mqttpath[30];
  char wifiSSID[30] = "Huu Tinh\0";
  char wifiPassword[30] = "bachai??@\0";

} configuration;

const char MQTT_HOST[] = "103.9.77.155";
String deviceId = String(App::getChipId(), 10);


WiFiClient net;
PubSubClient client(net);
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0 ;

time_t now;
time_t nowish = 1510592825;

void NTPConnect(void) {
  Serial.print("Setting time using SNTP");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  now = time(nullptr);
  while (now < nowish) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}


class MQTTHandler {

  private:
    static MQTTCallback callbackFunc;

  public:

    long lastReconnectMQTTAttempt = 0;

    void registerCallback(MQTTCallback callback) {
      callbackFunc = callback;
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

    }

    void loopReconnectMQTT() {
      uint32_t chipId = 0;
      for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }

      Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
      Serial.printf("This chip has %d cores\n", ESP.getChipCores());
      Serial.print("Chip ID: ");
      Serial.println(chipId);

      Serial.println("reconnecting Mqtt");

      long now = millis();
      if (now - lastReconnectMQTTAttempt > 60000) {

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
      uint32_t chipId = 0;
      for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }
      return chipId;
    }

    void connectMQTT() {

      if (client.connected()) {
        return;
      }

      NTPConnect();

      //      net.setTrustAnchors(&cert);
      //      net.setClientRSACert(&client_crt, &key);

      client.setServer(MQTT_HOST, 1883);
      client.setCallback(callback);

      Serial.println(MQTT_HOST);
      Serial.println(deviceId);

      Serial.println("Connecting to MQTT...");

      uint32_t chipId = getChipId();
      String clientId = String(chipId);

      if (client.connect(clientId.c_str())) {

        Serial.println("connected");
        App::sendSlackMessage();
        String jsonString = getInitialMessage(deviceId);
        client.publish(deviceId.c_str(), jsonString.c_str(), true);
        
        String switchTopic = deviceId + "/switch";
        client.subscribe(switchTopic.c_str(), 1);

        String switchonTopic = deviceId + "/switchon";
        client.subscribe(switchonTopic.c_str(), 1);

        String timeTriggerTopic = deviceId + "/timetrigger";
        client.subscribe(timeTriggerTopic.c_str(), 1);

        //    String jsonString = getStateMessage(relay);
        //    client.publish(deviceId.c_str(), jsonString.c_str(), true);
        client.subscribe(deviceId.c_str(), 1);

        String longlast = deviceId + "/longlast";
        client.subscribe(longlast.c_str(), 1);

      } else {

        Serial.print("failed with state ");
        Serial.print(client.state());
      }
    }

    void publish(const char* topic, const char* message, bool isRetained) {
      String switchonTopic = deviceId + "/" + topic;
      if (client.connected()) {
        client.publish(switchonTopic.c_str(), message, isRetained);
        Serial.print("Message published to topic: ");
        Serial.print(switchonTopic);
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