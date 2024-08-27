#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiHandler.h"
#include "time.h"

ESP8266WebServer server(80);

const uint8_t pinPir2 = D7;
int val;

WiFiHandler wifiHandler;


struct Configuration {

  char mqttServer[30];
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char mqttpath[30];
  char wifiSSID[30];
  char wifiPassword[30];

} configuration;

WiFiClient espClient;
PubSubClient client(espClient);

uint8_t ledPin = 17;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0 ;

void setup() {

  Serial.begin(115200);
  pinMode(pinPir2, INPUT);
//  pinMode(pinPir2, INPUT);

//      setupPir();
//
//  pinMode(pirPin2, INPUT);

  wifiHandler.setupWiFi();

}


void loop() {

  MDNS.update();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (WiFi.status() == WL_CONNECTED) {

    if (client.connected()) {

      client.loop();
    } else {

      checkPir();
    }
  } else {

    Serial.println("loopConnectWiFi");
    wifiHandler.loopConnectWiFi();
  }

  server.handleClient();
  delay(1000);

 
}

unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 5000; // 5 giây

void checkPir() {
  val = digitalRead(pinPir2);
  if (val == LOW) {
    Serial.println("No motion");
    delay(1000);
  } else {
    val = LOW;
    Serial.println("Motion detected  ALARM");

    // Kiểm tra thời gian để tránh gửi thông báo liên tục
    unsigned long currentTime = millis();
    if (currentTime - lastNotificationTime >= notificationInterval) {
      sendSlackMessage();
      lastNotificationTime = currentTime; // Cập nhật thời gian thông báo
    }

    delay(1000);
  }
}

void sendSlackMessage() {

  const char* webhookUrl = "http://103.9.77.155/devices/notify";

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient wifiClient;
    HTTPClient http;

    http.begin(wifiClient, webhookUrl);
    http.addHeader("Content-Type", "application/json");

    // Tạo đối tượng JSON
    time_t now = time(nullptr);
    DynamicJsonDocument jsonDoc(256);
    jsonDoc["id"] =  String(ESP.getChipId());
    jsonDoc["message"] = "Hello from esp8266";
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
