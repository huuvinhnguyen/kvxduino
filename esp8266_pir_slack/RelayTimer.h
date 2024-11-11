#include "WatchDog.h"
#include "Relay.h"
//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <memory>  // For std::unique_ptr
#include "time.h"

class RelayTimer {

    enum RepeatType { DAILY, WEEKLY, MONTHLY, NONE };

    struct Reminder {
      String startTime;
      int duration;
      RepeatType repeatType;
    };

  private:
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 7 * 3600;
    const int   daylightOffset_sec = 0 ;

    String reminderStartTime;
    int reminderDuration;
    String reminderRepeatType;

    std::vector<Reminder> reminders;


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

    RepeatType getRepeatTypeFromString(String repeatType) {
      if (repeatType == "daily") return DAILY;
      if (repeatType == "weekly") return WEEKLY;
      if (repeatType == "monthly") return MONTHLY;
      return NONE;
    }

  public:
    Relay relay;
    WatchDog watchDog;
    WiFiUDP ntpUDP;

    const long utcOffsetInSeconds = 7 * 3600;

    void setup() {
      relay.setup("");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    }

    void loop() {
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
      }

      for (const auto& reminder : reminders) {
        if (isReminderMatched(reminder.startTime, reminder.repeatType)) {
          Serial.println("Activate relay for reminder");
          if (reminder.duration > 0) {
            setSwitchOnLast(reminder.duration);
          } else {
            relay.setOn(true);
          }
        }
      }
      relay.loop([](int count) {});
    }

    void addReminder(String startTime, int duration, String repeatType) {

      RepeatType repeatTypeEnum = getRepeatTypeFromString(repeatType);

      // Search for an existing reminder with the same startTime
      for (auto& reminder : reminders) {
        if (reminder.startTime == startTime) {
          // Update the existing reminder's duration and repeatType
          reminder.duration = duration;
          reminder.repeatType = repeatTypeEnum;
          Serial.println("Updated existing reminder with new duration and repeatType.");
          return;  // Exit the method as we've updated the existing reminder
        }
      }

      // If no existing reminder is found, create a new one
      Reminder newReminder = { startTime, duration, repeatTypeEnum };
      reminders.push_back(newReminder);
      Serial.println("Added new reminder.");
    }

    void setReminder(String startTime, int duration, String repeatType) {
      reminderStartTime = startTime;
      reminderDuration = duration;
      reminderRepeatType = repeatType;
    }

    void removeReminder(String startTime, std::function<void()> callback) {
      // Find and erase the reminder with the matching startTime
      auto it = std::remove_if(reminders.begin(), reminders.end(),
      [&startTime](const Reminder & reminder) {
        return reminder.startTime == startTime;
      });

      // If a reminder was found and removed
      if (it != reminders.end()) {
        reminders.erase(it, reminders.end());  // Erase the reminder from the vector
        Serial.println("Reminder removed successfully.");
        callback();
      } else {
        Serial.println("No reminder found with the specified startTime.");
      }
    }

    bool isReminderMatched(String timestamp, RepeatType repeatType) {
      int year, month, day, hour, minute, second;
      if (timestamp.length() == 16) {
        timestamp += ":00";
      }
      sscanf(timestamp.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
      return isScheduleMatched(day, month, year, hour, minute, second, repeatType);
    }

    String getStateMessage(String deviceId, String topicType) {
      time_t now = time(nullptr);
      StaticJsonDocument<500> jsonDoc;
      jsonDoc["device_type"] = "switch";
      jsonDoc["topic_type"] = topicType;
      jsonDoc["device_id"] = deviceId;
      jsonDoc["switch_value"] = relay.value;
      jsonDoc["update_at"] = now;
      jsonDoc["longlast"] = relay.longlast;
      jsonDoc["timetrigger"] = watchDog.getTimeString();
      JsonObject reminder1 = jsonDoc.createNestedObject("reminder");
      reminder1["start_time"] = reminderStartTime;
      reminder1["duration"] = reminderDuration;
      reminder1["repeat_type"] = reminderRepeatType;

      JsonArray reminderArray = jsonDoc.createNestedArray("reminders");
      for (const auto& reminder : reminders) {
        JsonObject reminderJson = reminderArray.createNestedObject();
        reminderJson["start_time"] = reminder.startTime;
        reminderJson["duration"] = reminder.duration;
        reminderJson["repeat_type"] = reminder.repeatType == DAILY ? "daily" :
                                      reminder.repeatType == WEEKLY ? "weekly" :
                                      reminder.repeatType == MONTHLY ? "monthly" : "none";
      }

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
};
