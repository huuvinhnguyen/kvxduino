#include "WatchDog.h"
#include "Relay.h"
//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <memory>  // For std::unique_ptr
#include "time.h"

class RelayTimer {

  private:
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 7 * 3600;
    const int   daylightOffset_sec = 0 ;

  public:
    Relay relay;
    WatchDog watchDog;
    WiFiUDP ntpUDP;
    const long utcOffsetInSeconds = 7 * 3600;
    //    std::unique_ptr<NTPClient> timeClient;

    //    RelayTimer() : timeClient(nullptr) {}

    //    ~RelayTimer() {
    //      // Destructor to clean up resources
    //    }


    void setup() {
      //      timeClient = std::make_unique<NTPClient>(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
      //      timeClient->begin();
      relay.setup("");
    }

    void loopTriggerRelay() {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
      }

//      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

      bool isActive = watchDog.isAlarmAtTime(timeinfo.tm_hour, timeinfo.tm_min);
      if (isActive) {
        Serial.println("Activate relay");
        relay.switchOn();
      }

      relay.loop([](int count) {

      });
    }

    String getStateMessage(String deviceId) {
      time_t now = time(nullptr);  // Get the current epoch time
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["device_type"] = "switch";
      jsonDoc["device_id"] = deviceId;
      jsonDoc["value"] = relay.value;
      jsonDoc["update_at"] = now;
      jsonDoc["longlast"] = relay.longlast;
      jsonDoc["timetrigger"] = watchDog.getTimeString();
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      return jsonString;
    }

    void setSwitchOn() {

    }
};
