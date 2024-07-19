#include "Timer.h"
#include <NTPClient.h>
#include <WiFi.h>




bool WatchDog::isAlarmAtTime(int hours, int minutes) {

  String hourString;
  if (hours < 10) {
    hourString = "0" + String(hours);
  } else {
    hourString = String(hours);
  }

  String minuteString;
  if (minutes < 10) {
      minuteString = "0" + String(minutes);
  } else {
      minuteString = String(minutes);
  }

  String alarmTimeString;
  alarmTimeString += hourString;
  alarmTimeString += ".";
  alarmTimeString += minuteString;

  bool isExistAlarmTime = (this->timeString.indexOf(alarmTimeString) >= 0);
  
  if (isExistAlarmTime) {
    if (this->lastAlarmTimeString.equals(alarmTimeString)) {
      return false;
    } else {
      this->lastAlarmTimeString = alarmTimeString;
      return true;
    }
  }

  return false;
}
void WatchDog::setTimeString(String timeString) {
  this->timeString = timeString;
}

String WatchDog::getTimeString() {
  return this->timeString;
}
