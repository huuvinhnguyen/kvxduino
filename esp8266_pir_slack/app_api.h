#if defined(ESP32)
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif

#define TIMEOUT_MS 5000  // Thời gian chờ tối đa 5 giây


class AppApi {

  public:
    static constexpr const char* serverUrl = "http://103.9.77.155";
    static String deviceId;

    static String getDeviceId() {
      return AppApi::deviceId;
    }

    // Add any other common methods for your application here

    static void setup(String deviceId) {
      AppApi::deviceId = deviceId;
    }

    static void sendSlackMessage() {

      String url = String(AppApi::serverUrl) + "/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        http.setTimeout(TIMEOUT_MS);  // Đặt timeout cho HTTP request

        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] = AppApi::getDeviceId();
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

      String url = String(AppApi::serverUrl) + "/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;


        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] = AppApi::getDeviceId();
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

      String url = String(AppApi::serverUrl) + "/api/devices/receive_info";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, url);
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

      String url = String(AppApi::serverUrl) + "/api/devices/add_reminder";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, url);
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

      String url = String(AppApi::serverUrl) + "/devices/notify";

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient wifiClient;
        HTTPClient http;

        http.begin(wifiClient, url);
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

      String url = String(AppApi::serverUrl) + "/api/devices/device_info?device_id=" + deviceId;

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        WiFiClient client;

        http.begin(client, url);
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

    static void sendTrigger(String deviceId) {

      String url = String(AppApi::serverUrl) + "/api/devices/trigger";

      if (WiFi.status() == WL_CONNECTED) {

        Serial.println("No error connection");


        HTTPClient http;

        WiFiClient client;


        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");

        // Tạo đối tượng JSON
        http.setTimeout(TIMEOUT_MS);  // Đặt timeout cho HTTP request

        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["chip_id"] = deviceId;

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
        Serial.println("Error in WiFi connection 11111");
      }

    }

    static void switchRelayOn(String deviceId, int relayIndex, uint8_t switchValue) {

      String url = String(AppApi::serverUrl) + "/api/devices/switchon";

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient wifiClient;
        HTTPClient http;

        http.begin(wifiClient, url);
        http.addHeader("Content-Type", "application/json");

        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["device_id"] =  deviceId;
        jsonDoc["relay_index"] = relayIndex;
        jsonDoc["switch_value"] = switchValue;

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


    static void doUpdateOTA(String firmware_url) {
      Serial.println("Starting OTA update...");

      WiFiClientSecure client;
      client.setInsecure();

      HTTPClient http;
      http.begin(client, firmware_url);

      int httpCode = http.GET();
      if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return;
      }

      int contentLength = http.getSize();
      WiFiClient* stream = http.getStreamPtr();

      if (!Update.begin(contentLength, U_FLASH)) {
        Serial.println("Not enough space to begin OTA");
        Update.printError(Serial);
        return;
      }

      // Read data in chunks and write to flash
      size_t written = 0;
      uint8_t buff[1024];
      while (http.connected() && written < contentLength) {
        size_t len = stream->available();
        if (len) {
          int readLen = stream->readBytes(buff, min(len, sizeof(buff)));
          if (Update.write(buff, readLen) != readLen) {
            Serial.println("Failed to write chunk");
            Update.printError(Serial);
            return;
          }
          written += readLen;
          Serial.printf("Progress: %d / %d bytes\n", written, contentLength);
        }
        delay(1); // yield to watchdog
      }

      if (!Update.end()) {
        Serial.println("Error finishing update");
        Update.printError(Serial);
        return;
      }

      if (!Update.isFinished()) {
        Serial.println("Update not finished properly");
        return;
      }

      Serial.println("OTA Update complete. Rebooting...");
      delay(1000);
      ESP.restart();
    }

    static void updateLastSeen(int buildVersion, String appVersion) {

      String url = String(AppApi::serverUrl) + "/api/devices/update_last_seen";

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient client;

        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");

        // Tạo JSON payload với chip_id
        DynamicJsonDocument jsonDoc(128);
        jsonDoc["chip_id"] = AppApi::getDeviceId();  // ví dụ: "esp8266_5866822"
        jsonDoc["local_ip"] = WiFi.localIP();
        jsonDoc["build_version"] = buildVersion;
        jsonDoc["app_version"] = appVersion;
        Serial.println("app_version: ");
        Serial.println(appVersion);



        String payload;
        serializeJson(jsonDoc, payload);
        Serial.println("sending params: ");
        Serial.println(payload);



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
};

String AppApi::deviceId = "";
