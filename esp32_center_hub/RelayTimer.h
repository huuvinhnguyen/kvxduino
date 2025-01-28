#include "Relay.h"
//#include <NTPClient.h>
#include <WiFiUdp.h>
#include <memory>  // For std::unique_ptr
#include "time.h"

class RelayTimer {

    enum RepeatType { DAILY, WEEKLY, MONTHLY, NONE };

    struct Reminder {
      int relayIndex;
      String startTime;
      int duration;
      RepeatType repeatType;
      bool isActive;

    };

  private:
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 7 * 3600;
    const int   daylightOffset_sec = 0 ;

    std::vector<Reminder> reminders;
    std::vector<Relay> relays;

    std::vector<Reminder> getRemindersByRelayIndex(int relayIndex) {
      std::vector<Reminder> filteredReminders;

      for (const auto& reminder : reminders) {
        if (reminder.relayIndex == relayIndex) {
          filteredReminders.push_back(reminder);
        }
      }

      return filteredReminders;
    }

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
    WiFiUDP ntpUDP;

    const long utcOffsetInSeconds = 7 * 3600;
    using callbackFunc = std::function<void(String, int)>;

    callbackFunc cb2;

    void setup() {

      Relay relay1;
      relay1.setup(5); // Den truoc san
      relays.push_back(relay1);

      Relay relay2;
      relay2.setup(6);
      relays.push_back(relay2);

      Relay relay3;
      relay3.setup(7);
      relays.push_back(relay3);
      //
      Relay relay4;
      relay4.setup(8);
      relays.push_back(relay4);

      Relay relay5;
      relay5.setup(9);
      relays.push_back(relay5);

      Relay relay6;
      relay6.setup(2);
      relays.push_back(relay6);

      Relay relay7;
      relay7.setup(3);
      relays.push_back(relay7);

      Relay relay8;
      relay8.setup(4);
      relays.push_back(relay8);

      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    }

    void loop(callbackFunc func) {
      cb2 = func;
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
      }

      for (const auto& reminder : reminders) {
        if (isReminderMatched(reminder.startTime, reminder.repeatType) && reminder.isActive) {
          Serial.println("Activate relay for reminder");
          Serial.println("Duration: ");
          Serial.print(reminder.duration);
          if (reminder.duration > 0) {
            setSwitchOnLast(reminder.relayIndex, reminder.duration);
          } else {

            relays[reminder.relayIndex].setOn(true);
          }
        }
      }

      int index = 0;
      for (auto& relay : relays) {
        relay.loop([this, index](String state) {
          this->cb2(state, index);
        });
        index++;
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

    void setRemindersActive(int relayIndex, bool isActive) {
      relays[relayIndex].isRemindersActive = isActive;
      for (auto& reminder : reminders) {
        if (reminder.relayIndex == relayIndex) {
          reminder.isActive = isActive;
        }
      }
    }


    void removeReminder(int relayIndex, String startTime, std::function<void()> callback) {
      // Find and erase the reminder with the matching startTime
      auto it = std::remove_if(reminders.begin(), reminders.end(),
      [&relayIndex, &startTime](const Reminder & reminder) {
        return reminder.relayIndex == relayIndex && reminder.startTime == startTime;
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

    void setSwitchOnLast(int relayIndex, int longlast) {
      Serial.println("relayIndex: ");
      Serial.print(relayIndex);
      relays[relayIndex].longlast = longlast;
      relays[relayIndex].switchOn();
    }

    void setOn(int relayIndex, bool isOn) {
      relays[relayIndex].setOn(isOn);
    }

    void updateRelays(String deviceInfo) {
      DynamicJsonDocument jsonDoc(500);
      DeserializationError error = deserializeJson(jsonDoc, deviceInfo);
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
      }

      reminders.clear();
      JsonArray relaysArray = jsonDoc["device_info"]["relays"].as<JsonArray>();
      for (size_t i = 0; i < relaysArray.size(); i++) {
        JsonObject relayJson = relaysArray[i].as<JsonObject>();
        bool isOn = relayJson["switch_value"];
        relays[i].setOn(isOn);
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

    void handleMQTTCallback(String deviceId, char* topic, byte* payload, unsigned int length, std::function<void(StaticJsonDocument<500>, char*, String)> callback) {
      payload[length] = '\0';

      // Khởi tạo một bộ đệm để chứa payload
      char buffer[length + 1];
      memcpy(buffer, payload, length + 1);

      // Khởi tạo một object JSON và parse payload
      StaticJsonDocument<500> doc;
      DeserializationError error = deserializeJson(doc, buffer);

      // Kiểm tra lỗi parse
      if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
      }

      // Truy cập các trường trong object JSON
      const char* message = doc["message"];
      Serial.print("Received message: ");
      Serial.print(message);


      Serial.print("Message arrived in topic: ");
      Serial.println(topic);

      Serial.print("Message:");
      char *charArray = (char*)payload;
      String str = (String)charArray;
      Serial.print(str);



      if (strcmp(topic, deviceId.c_str()) == 0) {

        callback(doc, topic, "");

      }


      String pingTopic = deviceId + "/ping";
      if (strcmp(topic, pingTopic.c_str()) == 0) {
        String messageString = getStateMessage(deviceId, "ping");
        //        App::sendDeviceMessage(messageString);
        callback(doc, topic, messageString);

      }

      String switchOnTopic = deviceId + "/switchon";
      if (strcmp(topic, switchOnTopic.c_str()) == 0) {

        int relayIndex = doc["relay_index"];
        String action = doc["action"];
        if (action == "remove_reminder") {
          String startTime = doc["start_time"];

          removeReminder(relayIndex, startTime, [this, doc, topic, deviceId, callback]() {
            String messageString = this->getStateMessage(deviceId, "switchon");
            //            App::sendDeviceMessage(messageString);
            callback(doc, topic, messageString);
          });

        }

        if (doc.containsKey("longlast")) {
          int longlast = doc["longlast"];
          setSwitchOnLast(relayIndex, longlast);
          String messageString = getStateMessage(deviceId, "switchon");
          JsonArray relayIndexes = doc["relay_indexes"].as<JsonArray>();
          for (int index : relayIndexes) {
            setSwitchOnLast(index, longlast);
          }
          callback(doc, topic, messageString);

        }

        if (doc.containsKey("switch_value")) {
          bool isOn = doc["switch_value"];
          setOn(relayIndex, isOn);
          String messageString = getStateMessage(deviceId, "switchon");
          Serial.println("App::sendDeviceMessage(messageString)");
          Serial.println(messageString);
          JsonArray relayIndexes = doc["relay_indexes"].as<JsonArray>();
          for (int index : relayIndexes) {
            setOn(index, isOn);
          }
          callback(doc, topic, messageString);

        }

        if (doc.containsKey("is_reminders_active")) {

          bool isActive = doc["is_reminders_active"];
          setRemindersActive(relayIndex, isActive);
          String messageString = getStateMessage(deviceId, "switchon");
          Serial.println("App::sendDeviceMessage(messageString)");
          Serial.println(messageString);
          callback(doc, topic, messageString);

        }

        if (doc.containsKey("reminder")) {
          String startTime = doc["reminder"]["start_time"];
          int duration = doc["reminder"]["duration"];
          String repeatType = doc["reminder"]["repeat_type"];
          bool isRemindersActive = doc["is_reminders_active"];
          addReminder(relayIndex, startTime, duration, repeatType);

          String messageString = getStateMessage(deviceId, "switchon");
          //          App::addReminderMessage(messageString);
          callback(doc, topic, messageString);

        }
      }
    }

};
