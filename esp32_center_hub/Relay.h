#include <Arduino.h>

#define RELAY_ON HIGH  // Relay trạng thái bật
#define RELAY_OFF LOW  // Relay trạng thái tắt

class Relay {
  private:
    using callbackFunc = std::function<void(uint8_t)>;
    bool isSetOnLastingActive = false;
    unsigned long startAttempedTime = 0;
    uint8_t pin;

  public:
    void setup(uint8_t relayPin);
    void loop(callbackFunc func);
    void handleMessage(char *topic, String message);
    void setOn(bool isOn);
    void setLonglast(int seconds);
    void switchOn();
    uint8_t value = RELAY_ON;
    int longlast = 0; //miliseconds
    bool isRemindersActive = true;

};
