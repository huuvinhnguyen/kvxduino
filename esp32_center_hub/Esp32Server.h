#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "ViewInteractor.h"
#include <DNSServer.h>

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
    DNSServer dnsServer;


  public:
    WebServer server;
    WebSocketsServer webSocket = WebSocketsServer(81);

    Esp32Server() : server(80) {}

    void setup() {
      String ssid = "KVX" + String(ESP.getEfuseMac(), HEX);
      IPAddress apIP(192, 168, 4, 1); // Cố định IP AP
      IPAddress netMsk(255, 255, 255, 0);
      WiFi.softAPConfig(apIP, apIP, netMsk); // Set IP tĩnh cho AP
      WiFi.softAP(ssid);

      // Khởi động DNS Server để redirect mọi request
      dnsServer.start(53, "*", apIP);

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

    void timing(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec) {

      DynamicJsonDocument jsonDoc(256);
      jsonDoc["year"] = year;
      jsonDoc["month"] = month;
      jsonDoc["date"] = date;
      jsonDoc["hour"] = hour;
      jsonDoc["minute"] = min;
      jsonDoc["second"] = sec;

      String jsonString;
      serializeJson(jsonDoc, jsonString);
      webSocket.broadcastTXT(jsonString);

      // Update server time
      serverTime.year = year;
      serverTime.month = month;
      serverTime.date = date;
      serverTime.hour = hour;
      serverTime.minute = min;
      serverTime.second = sec;

      Serial.println("jsonString: " + jsonString);
    }

    void updateDeviceInfo(String deviceInfo) {
      this->deviceInfo = deviceInfo;
    }

    void setupFilesServer() {
      ViewInteractor viewInteractor;
      viewInteractor.lookupFiles();
      typedef std::function<void(ServerTime)> callbackFunc;


      server.onNotFound([this, &viewInteractor]() {
        String path = server.uri();

        if (path == "/generate_204" || path == "/hotspot-detect.html" || path == "/ncsi.txt" || path == "/") {
          // Những request mà iPhone dùng để check captive portal
//          server.send(200, "text/html", "<html><body><h1>Captive Portal</h1></body></html>");
            viewInteractor.handleRoot(&server);
        } else {
          ViewInteractor viewInteractor;
          if (!viewInteractor.isFileRead(path))
            server.send(404, "text/plain", "FileNotFound");
          else {
            File file = viewInteractor.getFileRead(path);
            size_t sent = server.streamFile(file, viewInteractor.getContentType(path));
            file.close();
          }
        }
      });


      server.on("/", [this, &viewInteractor]() {
        viewInteractor.handleRoot(&server);
      });

      server.on("/get_time", HTTP_POST, [this]() {
        this->handleGetTime();
      });

      server.on("/set_time", HTTP_POST, [this]() {
        this->handleSetTime();
      });


      server.on("/add_reminder", HTTP_POST, [this]() {
        this->handleSetReminder();
      });

      server.on("/switchon", HTTP_POST, [this]() {
        this->handleSwitchOn();
      });

      server.on("/get_device_info", HTTP_POST, [this]() {
        this->handleGetDeviceInfo();
      });

      server.on("/remove_reminders", HTTP_POST, [this]() {
        this->handleRemoveAllReminders();
      });

      server.on("/set_reminders_active", HTTP_POST, [this]() {
        this->handleSetRemindersActive();
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

    void handleSetReminder() {

      if (server.hasArg("plain") == false) {
        server.send(400, "application/json", "{\"error\":\"No data received\"}");
        return;
      }

      // Đọc JSON từ request
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, server.arg("plain"));

      if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      int relayIndex = doc["relay_index"];
      String startTime = doc["reminder"]["start_time"];
      int duration = doc["reminder"]["duration"];
      String repeatType = doc["reminder"]["repeat_type"];
      bool isRemindersActive = doc["is_reminders_active"];
      Serial.println("handleSetReminder");


      setReminderCallbackFunc(relayIndex, startTime, duration, repeatType);

      server.send(200, "application/json", "{\"status\":\"Time updated\"}");
    }



    void handleGetDeviceInfo() {
      server.send(200, "application/json", deviceInfo);
      Serial.println("deviceInfo: ");
      Serial.println(deviceInfo);

    }

    void handleRemoveAllReminders() {
      removeAllRemindersCallbackFunc();
      server.send(200, "application/json", "{\"status\":\"removeAllRemindersCallbackFunc did activate\"}");
    }



    void registerCallback(ServerSetTimeCallback callback) {
      setTimeCallbackFunc = callback;
    }

    void registerReminderCallback(ServerSetReminderCallback callback) {
      setReminderCallbackFunc = callback;
    }

    void registerSwitchOnLonglastCallback(ServerSwitchOnLonglastCallback callback) {
      switchOnLonglastCallbackFunc = callback;
    }

    void registerSwitchOnCallback(ServerSwitchOnCallback callback) {
      switchOnCallbackFunc = callback;
    }

    void registerRemoveAllReminders(ServerRemoveAllRemindersCallback callback) {
      removeAllRemindersCallbackFunc = callback;
    }

    void registerSetRemindersActive(ServerSetRemindersActiveCallback callback) {
      setRemindersActiveCallbackFunc = callback;
    }

    const int max_ws_client = 5;
    int wsClientNumber[5] = { -1, -1, -1, -1, -1};
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
      switch (type) {
        case WStype_DISCONNECTED:
          DBG_OUTPUT_PORT.printf("[%u] Disconnected!n", num);
          break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            DBG_OUTPUT_PORT.printf("[%u] Connected from %d.%d.%d.%d url: %sn", num, ip[0], ip[1], ip[2], ip[3], payload);

            int index = (num % max_ws_client);
            if (index <= 0) index = 0;
            wsClientNumber[index] = num;
            DBG_OUTPUT_PORT.printf("Save client index %d :%un", index, num);
            // send message to client

          }
          break;
        case WStype_TEXT:
          DBG_OUTPUT_PORT.printf("[%u] get Text: %sn", num, payload);
          if (payload[0] == '1') {
            //            setStatus(true);

          }
          else if (payload[0] == '0') {
            //            setStatus(false);

          }

          break;
      }

    }
};



ServerSetTimeCallback Esp32Server::setTimeCallbackFunc = nullptr;
ServerSetReminderCallback Esp32Server::setReminderCallbackFunc = nullptr;
ServerSwitchOnLonglastCallback Esp32Server::switchOnLonglastCallbackFunc = nullptr;
ServerSwitchOnCallback Esp32Server::switchOnCallbackFunc = nullptr;
ServerRemoveAllRemindersCallback Esp32Server::removeAllRemindersCallbackFunc = nullptr;
ServerSetRemindersActiveCallback Esp32Server::setRemindersActiveCallbackFunc = nullptr;
