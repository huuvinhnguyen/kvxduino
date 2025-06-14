#include <DS1302.h>
#include "time.h"
#include "DataDefault.h"

class TimeClock {
  private:

    enum RepeatType { DAILY, WEEKLY, MONTHLY, NONE };

    struct Reminder {
      int relayIndex;
      String startTime;
      int duration;
      RepeatType repeatType;
      bool isActive;

    };

    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 7 * 3600;
    const int   daylightOffset_sec = 0 ;
    DataDefault<String> dataDefault;

    RepeatType getRepeatTypeFromString(String repeatType) {
      if (repeatType == "daily") return DAILY;
      if (repeatType == "weekly") return WEEKLY;
      if (repeatType == "monthly") return MONTHLY;
      return NONE;
    }

    std::vector<Reminder> getRemindersByRelayIndex(int relayIndex) {
      std::vector<Reminder> filteredReminders;

      for (const auto& reminder : reminders) {
        if (reminder.relayIndex == relayIndex) {
          filteredReminders.push_back(reminder);
        }
      }

      return filteredReminders;
    }


  public:
    typedef std::function<void(Time)> callbackFunc;

    const int CE_PIN = 3; // RST
    const int IO_PIN = 4; // DATA
    const int SCLK_PIN = 5; // CLOCk
    DS1302 rtc;  // Khai báo biến rtc mà không khởi tạo trực tiếp

    TimeClock() : rtc(CE_PIN, IO_PIN, SCLK_PIN) {}

    std::vector<Reminder> reminders;
    std::vector<Relay> relays;

    void setup() {

      rtc.halt(false);
      rtc.writeProtect(false);

      Relay relay1;
      //      relay1.setup(2); 
      //      relay1.setup(4); 
      //      relay1.setup(5); 
      relay1.setup(2); // D8

      relays.push_back(relay1);

      dataDefault.setup();

      String deviceInfo = dataDefault.readEEPROMString(0);
      if (deviceInfo.isEmpty()) {} else {
        updateRelays(deviceInfo);
      }
    }

    String getStateMessage() {
      time_t now = time(nullptr);
      StaticJsonDocument<500> jsonDoc;
      jsonDoc["device_type"] = "switch";
      jsonDoc["update_at"] = now;

      JsonArray relayArray = jsonDoc.createNestedArray("relays");
      for (size_t relayIndex = 0; relayIndex < relays.size(); ++relayIndex) {
        const auto& relay = relays[relayIndex];
        JsonObject relayJson = relayArray.createNestedObject();
        relayJson["switch_value"] = relay.value;
        relayJson["longlast"] = relay.longlast;
        relayJson["is_reminders_active"] = relay.isRemindersActive;

        // Filter reminders for the current relay's index
        std::vector<Reminder> filteredReminders = getRemindersByRelayIndex(relayIndex);

        JsonArray reminderArray = relayJson.createNestedArray("reminders");
        for (const auto& reminder : filteredReminders) {
          JsonObject reminderJson = reminderArray.createNestedObject();
          reminderJson["start_time"] = reminder.startTime;
          reminderJson["duration"] = reminder.duration;
          reminderJson["repeat_type"] = reminder.repeatType == DAILY ? "daily" :
                                        reminder.repeatType == WEEKLY ? "weekly" :
                                        reminder.repeatType == MONTHLY ? "monthly" : "none";
        }
      }

      String jsonString;
      serializeJson(jsonDoc, jsonString);
      return jsonString;
    }

    void setTime(int year, int month, int day, int hour, int minute, int second) {

      Time t(year, month, day, hour, minute, second, Time::kSunday);
      rtc.time(t);

    }

    bool isReminderMatched(String timestamp, RepeatType repeatType) {
      int year, month, day, hour, minute, second;
      if (timestamp.length() == 16) {
        timestamp += ":00";
      }
      sscanf(timestamp.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
      return isScheduleMatched(day, month, year, hour, minute, second, repeatType);
    }

    bool isScheduleMatched(int day, int month, int year, int hour, int minute, int second, RepeatType repeatType) {
      Time t = rtc.time(); // Lấy thời gian từ DS1302 RTC

      if (t.hr != hour || t.min != minute || t.sec != second) {
        return false;  // Nếu giờ, phút hoặc giây không khớp, kết thúc ngay
      }

      // Kiểm tra kiểu lặp
      switch (repeatType) {
        case DAILY:
          return true; // Lặp hàng ngày chỉ cần khớp giờ, phút, giây

        case WEEKLY:
          return (t.day == day); // Kiểm tra ngày trong tuần

        case MONTHLY:
          return (t.date == day); // Kiểm tra ngày trong tháng

        case NONE:
          // Kiểm tra đầy đủ năm, tháng, ngày, giờ, phút, giây
          return (t.yr == year &&
                  t.mon == month &&
                  t.date == day &&
                  t.hr == hour &&
                  t.min == minute &&
                  t.sec == second);
      }
      return false;
    }

    void loop(callbackFunc func) {
      Serial.println("sssssssssssss");
      Time t = rtc.time();
      func(t);

      Serial.print("Year: ");
      Serial.print(t.yr);
      Serial.print(" Month: ");
      Serial.print(t.mon);
      Serial.print(" Day: ");
      Serial.print(t.date);
      Serial.print(" Hour: ");
      Serial.print(t.hr);
      Serial.print(" Minute: ");
      Serial.print(t.min);
      Serial.print(" Second: ");
      Serial.print(t.sec);
      Serial.print(" Weekday: ");
      Serial.println(t.day);  // Số từ 0 (Chủ Nhật) đến 6 (Thứ Bảy)


      for (const auto& reminder : reminders) {
        if (isReminderMatched(reminder.startTime, reminder.repeatType) && reminder.isActive) {
          Serial.println("Activate relay for reminder");
          Serial.println("Duration: ");
          Serial.print(reminder.duration);
          if (reminder.duration > 0) {
            setSwitchOnLast(reminder.relayIndex, reminder.duration);
          } else {
            Serial.println("Reminder set ON");
            relays[reminder.relayIndex].setOn(true);
          }
        }
      }

      int index = 0;
      for (auto& relay : relays) {
        relay.loop([this, index]( uint8_t value) {
          
        });
        index++;
      }
    }

    void setOn(int relayIndex, bool isOn) {
      Serial.println("Relay index:" );
      Serial.print(relayIndex);
      Serial.println("is On: ");
      Serial.print(isOn);

      relays[relayIndex].setOn(isOn);
    }

    void setSwitchOnLast(int relayIndex, int longlast) {
      Serial.println("relayIndex: ");
      Serial.print(relayIndex);
      Serial.println("longlast: " + longlast);
      relays[relayIndex].longlast = longlast;
      relays[relayIndex].switchOn();
    }

    void updateRelays(String deviceInfo) {

      Serial.println("Updating relays...");
      DynamicJsonDocument jsonDoc(500);
      DeserializationError error = deserializeJson(jsonDoc, deviceInfo);
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
      }

      reminders.clear();

      JsonArray relaysArray = jsonDoc["relays"].as<JsonArray>();
      String jsonString;
      serializeJson(relaysArray, jsonString);
      Serial.println("jsonDoc: " + jsonString);
      for (size_t i = 0; i < relaysArray.size(); i++) {
        JsonObject relayJson = relaysArray[i].as<JsonObject>();
        bool isOn = relayJson["switch_value"];
        relays[i].setOn(isOn);
        Serial.println("relay is on: " + isOn);
        if (relayJson.containsKey("is_reminders_active")) {
          relays[i].isRemindersActive = relayJson["is_reminders_active"];
        }

        JsonArray remindersArray = relayJson["reminders"].as<JsonArray>();

        for (JsonObject reminderJson : remindersArray) {
          int relayIndex = i;
          String startTime = reminderJson["start_time"].as<String>();
          int duration = reminderJson["duration"];
          String repeatType = reminderJson["repeat_type"].as<String>();
          RepeatType repeatTypeEnum = getRepeatTypeFromString(repeatType);

          Reminder newReminder = { relayIndex, startTime, duration, repeatTypeEnum, relays[relayIndex].isRemindersActive };
          reminders.push_back(newReminder);

        }
      }
    }

    void addReminder(int relayIndex, String startTime, int duration, String repeatType) {

      RepeatType repeatTypeEnum = getRepeatTypeFromString(repeatType);

      // Search for an existing reminder with the same startTime
      for (auto& reminder : reminders) {
        if (reminder.relayIndex == relayIndex && reminder.startTime == startTime) {
          // Update the existing reminder's duration and repeatType
          reminder.duration = duration;
          reminder.repeatType = repeatTypeEnum;
          Serial.println("Updated existing reminder with new duration and repeatType.");

          return;  // Exit the method as we've updated the existing reminder
        }
      }

      // If no existing reminder is found, create a new one
      Reminder newReminder = { relayIndex, startTime, duration, repeatTypeEnum, true };
      reminders.push_back(newReminder);
      Serial.println("Added new reminder.");

    }

    void saveReminderData() {

      String deviceInfo = getStateMessage();
      dataDefault.writeEEPROMString(0, deviceInfo, deviceInfo.length() + 1);

      Serial.println("Reminder Data: " + deviceInfo);

      Serial.println("Đã ghi chuỗi vào EEPROM");
    }

    void removeAllReminders() {
      reminders.clear();
      dataDefault.clearEEPROMString(0);
      String deviceInfo = getStateMessage();
      updateRelays(deviceInfo);
      Serial.println("Data Default : Removed all reminders");
    }

    void setRemindersActive(int relayIndex, bool isActive) {
      relays[relayIndex].isRemindersActive = isActive;
      for (auto& reminder : reminders) {
        if (reminder.relayIndex == relayIndex) {
          reminder.isActive = isActive;
        }
      }
    }
};
