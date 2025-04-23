#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "ViewInteractor.h"

using ServerSetTimeCallback = void(*)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
using ServerSetReminderCallback = void(*)(int relayIndex, String startTime, int duration, String repeatType);
using ServerSwitchOnLonglastCallback = void(*)(int relayIndex, int longlast);
using ServerSwitchOnCallback = void(*)(int relayIndex, bool isOn);
using ServerRemoveAllRemindersCallback = void(*)(void);
using ServerSetRemindersActiveCallback = void(*)(int relayIndex, bool isActive);

class Esp32Server {
  struct ServerTime {
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
  };

private:
  static ServerSetTimeCallback setTimeCallbackFunc;
  static ServerSetReminderCallback setReminderCallbackFunc;
  static ServerSwitchOnLonglastCallback switchOnLonglastCallbackFunc;
  static ServerSwitchOnCallback switchOnCallbackFunc;
  static ServerRemoveAllRemindersCallback removeAllRemindersCallbackFunc;
  static ServerSetRemindersActiveCallback setRemindersActiveCallbackFunc;
  ServerTime serverTime;
  String deviceInfo;

public:
  WebServer server;
  WebSocketsServer webSocket = WebSocketsServer(81);

  Esp32Server() : server(80) {}

  void setup() {
    String ssid = "KVX" + String(ESP.getEfuseMac(), HEX);
    WiFi.softAP(ssid.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    setupFilesServer();
    server.begin();
    webSocket.begin();
  }

  void loop() {
    server.handleClient();
    webSocket.loop();
  }

  void setupFilesServer() {
    server.on("/", HTTP_GET, [this]() {
      server.send(200, "text/html", "<h1>ESP32 Web Server</h1>");
    });

    server.on("/set_time", HTTP_POST, [this]() {
      handleSetTime();
    });

    server.on("/set_reminders_active", HTTP_POST, [this]() {
      handleSetRemindersActive();
    });

    server.on("/switchon", HTTP_POST, [this]() {
      handleSwitchOn();
    });

    server.on("/get_time", HTTP_GET, [this]() {
      handleGetTime();
    });

    server.on("/get_device_info", HTTP_GET, [this]() {
      server.send(200, "application/json", deviceInfo);
    });
  }

  void handleSetTime() {
    if (server.hasArg("plain")) {
      DynamicJsonDocument doc(256);
      deserializeJson(doc, server.arg("plain"));

      int year = doc["year"];
      int month = doc["month"];
      int day = doc["day"];
      int hour = doc["hour"];
      int minute = doc["minute"];
      int second = doc["second"];

      setTimeCallbackFunc(year, month, day, hour, minute, second);
      server.send(200, "application/json", "{\"status\":\"Time updated\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"No data received\"}");
    }
  }

  void handleSetRemindersActive() {
    if (server.hasArg("plain")) {
      DynamicJsonDocument doc(256);
      deserializeJson(doc, server.arg("plain"));

      int relayIndex = doc["relay_index"];
      bool isActive = doc["is_reminders_active"];
      setRemindersActiveCallbackFunc(relayIndex, isActive);
      server.send(200, "application/json", "{\"status\":\"Reminders updated\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"No data received\"}");
    }
  }

  void handleSwitchOn() {
    if (server.hasArg("plain")) {
      DynamicJsonDocument doc(256);
      deserializeJson(doc, server.arg("plain"));

      int relayIndex = doc["relay_index"];
      bool isOn = doc["switch_value"];
      switchOnCallbackFunc(relayIndex, isOn);
      server.send(200, "application/json", "{\"status\":\"Switch updated\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"No data received\"}");
    }
  }

  void handleGetTime() {
    DynamicJsonDocument doc(256);
    doc["year"] = serverTime.year;
    doc["month"] = serverTime.month;
    doc["date"] = serverTime.date;
    doc["hour"] = serverTime.hour;
    doc["minute"] = serverTime.minute;
    doc["second"] = serverTime.second;

    String jsonString;
    serializeJson(doc, jsonString);
    server.send(200, "application/json", jsonString);
  }
};

ServerSetTimeCallback Esp32Server::setTimeCallbackFunc = nullptr;
ServerSetReminderCallback Esp32Server::setReminderCallbackFunc = nullptr;
ServerSwitchOnLonglastCallback Esp32Server::switchOnLonglastCallbackFunc = nullptr;
ServerSwitchOnCallback Esp32Server::switchOnCallbackFunc = nullptr;
ServerRemoveAllRemindersCallback Esp32Server::removeAllRemindersCallbackFunc = nullptr;
ServerSetRemindersActiveCallback Esp32Server::setRemindersActiveCallbackFunc = nullptr;
