#include "WatchDog.h"
#include "Relay.h"
//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <memory>  // For std::unique_ptr
#include "time.h"

class RelayTimer {

    enum RepeatType { DAILY, WEEKLY, MONTHLY, NONE };

  private:
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 7 * 3600;
    const int   daylightOffset_sec = 0 ;

    String reminderTimeStart;
    int reminderDuration;
    String reminderRepeatType;

    bool isScheduleMatched(int day, int month, int year, int hour, int minute, int second, RepeatType repeatType) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        if (timeinfo.tm_hour != hour || timeinfo.tm_min != minute || timeinfo.tm_sec != second) {
          return false;  // Nếu giờ hoặc phút không khớp, kết thúc ngay
        }

        // Kiểm tra kiểu lặp
        switch (repeatType) {
          case DAILY:
            return true; // Nếu là lặp hàng ngày, chỉ cần khớp giờ và phút

          case WEEKLY:
            return (timeinfo.tm_wday == day); // Kiểm tra ngày trong tuần

          case MONTHLY:
            return (timeinfo.tm_mday == day); // Kiểm tra ngày trong tháng

          case NONE:
            // Kiểm tra đầy đủ ngày, tháng, năm, giờ, phút để không lặp lại
            return (timeinfo.tm_year + 1900 == year &&
                    (timeinfo.tm_mon + 1) == month &&
                    timeinfo.tm_mday == day &&
                    timeinfo.tm_hour == hour &&
                    timeinfo.tm_min == minute &&
                    timeinfo.tm_sec == second);
        }
      }
      return false;
    }

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

    void loop() {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
      }

      //      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

      //      bool isActive = watchDog.isAlarmAtTime(timeinfo.tm_hour, timeinfo.tm_min);
      bool isActive = isReminderMatched(reminderTimeStart, reminderRepeatType);

      if (isActive) {
        Serial.println("Activate relay");
        if (reminderDuration > 0) {
          Serial.println("Duration: ");
          Serial.print(reminderDuration);
          setSwitchOnLast(reminderDuration);
        } else {
          relay.switchOn();

        }
      }

      relay.loop([](int count) {

      });
    }

    void setReminder(String timeStart, int duration, String repeatType) {
      reminderTimeStart = timeStart;
      reminderDuration = duration;
      reminderRepeatType = repeatType;
    }

    String getStateMessage(String deviceId, String topicType) {
      time_t now = time(nullptr);  // Get the current epoch time
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["device_type"] = "switch";
      jsonDoc["topic_type"] = topicType;
      jsonDoc["device_id"] = deviceId;
      jsonDoc["switch_value"] = relay.value;
      jsonDoc["update_at"] = now;
      jsonDoc["longlast"] = relay.longlast;
      jsonDoc["timetrigger"] = watchDog.getTimeString();
      JsonObject reminder = jsonDoc.createNestedObject("reminder");
      reminder["time_start"] = reminderTimeStart;
      reminder["duration"] = reminderDuration;
      reminder["repeat_type"] = reminderRepeatType;
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      return jsonString;
    }

    void setSwitchOnLast(int longlast) {
      relay.longlast = longlast;
      relay.switchOn();
    }

    void setOn(bool isOn) {
      relay.setOn(isOn);
    }

    bool isReminderMatched(String timestamp, String stringRepeatType) {
      int year, month, day, hour, minute, second;
      sscanf(timestamp.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
      RepeatType repeatType;

      // Xác định kiểu lặp
      if (stringRepeatType == "daily") {
        repeatType = DAILY;
      } else if (stringRepeatType == "weekly") {
        repeatType = WEEKLY;
      } else if (stringRepeatType == "monthly") {
        repeatType = MONTHLY;
      } else if (stringRepeatType == "none") {
        repeatType = NONE;  // Thêm kiểu NONE để xử lý không lặp lại
      } else {
        repeatType = NONE; // Mặc định là DAILY nếu không xác định rõ ràng
      }

      return isScheduleMatched(day, month, year, hour, minute, second, repeatType);
    }



};
