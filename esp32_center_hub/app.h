#include <HTTPClient.h>

class App {

  public:
    // Method to retrieve Chip ID
    static uint32_t getChipId() {
      uint32_t chipId = 0;
      for (int i = 0; i < 17; i += 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }
      return chipId;
    }

    // Add any other common methods for your application here

    static void sendSlackMessage() {

      uint32_t chipId = 0;
      for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
      }
      const char* webhookUrl = "http://103.9.77.155/devices/notify";
    
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
    
        http.begin(webhookUrl);
        http.addHeader("Content-Type", "application/json");
    
        // Tạo đối tượng JSON
        time_t now = time(nullptr);
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["id"] = chipId;
        jsonDoc["message"] = "Hello from esp32";
        jsonDoc["time"] = now;
        jsonDoc["model"] = "esp32";
    
    
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
};
