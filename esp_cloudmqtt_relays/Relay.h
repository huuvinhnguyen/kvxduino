#include <Arduino.h>
#include <ESP8266WiFi.h>

class Relay {
  private:
    using callbackFunc = void (*) (int);
    uint8_t relayPins[4] = {5, 4, 17, 2};
    bool isSetOnLastingActive = false;
    long startAttempedTime = 0;
    int longlast = 15;
    String path;
    
  public:
    void setup(String mqttPath);
    void loop(callbackFunc func);
    void handleMessage(char *topic, String message);
    void setOn(bool isOn);
    callbackFunc cb1;
    void setLonglast(int seconds);
    void switchOn();
};
