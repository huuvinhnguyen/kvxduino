#include <Arduino.h>

class Relay {
  private:
    using callbackFunc = void (*) (int);
//    uint8_t relayPins[5] = {D5};
    bool isSetOnLastingActive = false;
    long startAttempedTime = 0;
    uint8_t pin;

  public:
    void setup(uint8_t relayPin);
    void loop(callbackFunc func);
    void handleMessage(char *topic, String message);
    void setOn(bool isOn);
    callbackFunc cb1;
    void setLonglast(int seconds);
    void switchOn();
    uint8_t value = LOW;
    int longlast = 0; //miliseconds
    bool isRemindersActive = true;

};
