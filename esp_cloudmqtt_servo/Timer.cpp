#include "Timer.h"

bool WatchDog::isAlarmAtTime(int hours, int minutes) {

  String hourString = String(hours);
  String minuteString = String(minutes);
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
