#include <Arduino.h>

#define RELAY_ON LOW  // Relay trạng thái bật
#define RELAY_OFF HIGH  // Relay trạng thái tắt

class Relay {
  private:
    using callbackFunc = std::function<void(String, uint8_t)>;
    bool isSetOnLastingActive = false;
    unsigned long startAttempedTime = 0;
    uint8_t pin;

  public:
    void setup(uint8_t relayPin);
    void loop(callbackFunc func);
    void handleMessage(char *topic, String message);
    void setOn(bool isOn);
    callbackFunc cb1;
    void setLonglast(int seconds);
    void switchOn();
    uint8_t value = RELAY_OFF;
    int longlast = 0; //miliseconds
    bool isRemindersActive = true;

};
