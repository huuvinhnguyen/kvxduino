#include <PubSubClient.h>
#include <WiFiClient.h>

//#include "app.h"

//using MQTTCallback = void(*)(char*, byte*, unsigned int);
using MQTTCallback = void(*)(char* topic, byte* payload, unsigned int length);
using MQTTDidFinishConnectCallback = void(*)(void);



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


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0 ;

time_t now;
time_t nowish = 1510592825;

void NTPConnect(void) {
  Serial.print("Setting time using SNTP");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  unsigned long startMillis = millis(); // Record the start time
  const unsigned long timeout = 10000;  // 10 seconds in milliseconds

  now = time(nullptr);
  while (now < nowish) {
    if (millis() - startMillis >= timeout) {
      Serial.println("Failed to set time. Restarting...");
      ESP.restart(); // Restart the ESP8266
      return; // Exit the function after restart command
    }
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
    WiFiClient net;
    PubSubClient client;
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
    //    String deviceId = String(ESP.getChipId());
    String deviceId = "esp8266_" + String(ESP.getChipId());

    MQTTHandler() : net(), client(net) {} // Khởi tạo PubSubClient với WiFiClient
    long lastReconnectMQTTAttempt = 0;

    void setup(String deviceId) {
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
      return ESP.getChipId();
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

      //      uint32_t chipId = getChipId();
      String clientId = generateRandomUUID();


      if (client.connect(clientId.c_str())) {

        Serial.println("connected");

        String jsonString = getInitialMessage(deviceId);
        client.publish(deviceId.c_str(), jsonString.c_str(), false);

        String switchTopic = deviceId + "/switch";
        client.subscribe(switchTopic.c_str(), 1);

        String switchonTopic = deviceId + "/switchon";
        client.subscribe(switchonTopic.c_str(), 1);

        String timeTriggerTopic = deviceId + "/timetrigger";
        client.subscribe(timeTriggerTopic.c_str(), 1);

        String longlast = deviceId + "/longlast";
        client.subscribe(longlast.c_str(), 1);

        String ping = deviceId + "/ping";
        client.subscribe(ping.c_str(), 1);

        String refresh = deviceId + "/refresh";
        client.subscribe(refresh.c_str(), 1);

        String restart = deviceId + "/restart";
        client.subscribe(restart.c_str(), 1);

        didFinishConnectCallbackFunc();

      } else {

        Serial.print("failed with state ");
        Serial.print(client.state());
      }
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
