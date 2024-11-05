#include <Arduino.h>

class Relay {
  private:
    using callbackFunc = void (*) (int);
    uint8_t relayPins[2] = {D4, D5};
    bool isSetOnLastingActive = false;
    long startAttempedTime = 0;
    String path;

  public:
    void setup(String mqttPath);
    void loop(callbackFunc func);
    void handleMessage(char *topic, String message);
    void setOn(bool isOn);
    callbackFunc cb1;
    void setLonglast(int seconds);
    void switchOn();
    uint8_t value = LOW;
    int longlast = 0; //miliseconds

};
