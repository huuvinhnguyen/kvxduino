#include <Arduino.h>



class WatchDog {

  private:
    String timeString;
    String lastAlarmTimeString = "";

  public:
    
    void setTimeString(String timeString);
    bool isAlarmAtTime(int hours, int minutes);
};
