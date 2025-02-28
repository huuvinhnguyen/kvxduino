#if defined(ESP32)
#include <HTTPClient.h>

#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif


class App {

  public:
    static uint32_t getChipId() {
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
    static String getDeviceId() {
      return "esp32_" + String(getChipId());
    }





    // Add any other common methods for your application here

    static void sendSlackMessage() {


      const char* webhookUrl = "http://103.9.77.155/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;


        http.begin(client, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] = App::getDeviceId();
        jsonDoc["message"] = "xin_chao";
        jsonDoc["time"] = now;
        jsonDoc["model"] = "esp8266";


        // Chuyển đổi đối tượng JSON thành chuỗi
        String payload;
        serializeJson(jsonDoc, payload);

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
    }

    static void sendSlackMessage(String state, int index) {


      const char* webhookUrl = "http://103.9.77.155/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;


        http.begin(client, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] = App::getDeviceId();
        jsonDoc["message"] = "xin_chao!";
        jsonDoc["time"] = now;
        jsonDoc["model"] = "esp8266";
        jsonDoc["relay_index"] = index;
        jsonDoc["relay_state"] = state;



        // Chuyển đổi đối tượng JSON thành chuỗi
        String payload;
        serializeJson(jsonDoc, payload);

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
    }

    static void sendDeviceMessage(String messagePayload) {

      const char* webhookUrl = "http://103.9.77.155/api/devices/receive_info";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(messagePayload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
    }

    static void addReminderMessage(String messagePayload) {

      const char* webhookUrl = "http://103.9.77.155/api/devices/add_reminder";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(messagePayload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
    }

    static void sendPirSlackMessage(String deviceId) {

      const char* webhookUrl = "http://103.9.77.155/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient wifiClient;
        HTTPClient http;

        http.begin(wifiClient, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] =  deviceId;
        jsonDoc["message"] = "xin_chao";
        jsonDoc["time"] = now;
        jsonDoc["model"] = "esp8266";
        jsonDoc["sen"] = "pir";


        // Chuyển đổi đối tượng JSON thành chuỗi
        String payload;
        serializeJson(jsonDoc, payload);

        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
    }

    static String getDeviceInfo(String deviceId) {

      const String webhookUrl = "http://103.9.77.155/api/devices/device_info?device_id=" + deviceId;

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, webhookUrl);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response Code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
          return String(response);
        } else {
          Serial.println("Error code: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("Error in WiFi connection");
      }
      return  "";
    }
};
