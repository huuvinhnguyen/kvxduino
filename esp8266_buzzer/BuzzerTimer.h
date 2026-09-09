#pragma once

#include <Arduino.h>

#define BUZZER_ON HIGH
#define BUZZER_OFF LOW

class BuzzerTimer {
  private:
    uint8_t pin;
    bool isActive = false;
    unsigned long startedAt = 0;
    unsigned long duration = 0;

  public:
    void setup(uint8_t buzzerPin) {
      pin = buzzerPin;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, BUZZER_OFF);
    }

    void loop() {
      if (!isActive || duration == 0) {
        return;
      }

      if (millis() - startedAt >= duration) {
        setOn(false);
      }
    }

    void setOn(bool isOn) {
      digitalWrite(pin, isOn ? BUZZER_ON : BUZZER_OFF);
      isActive = false;
    }

    void activateFor(int longlast) {
      duration = longlast;
      startedAt = millis();
      isActive = true;
      digitalWrite(pin, BUZZER_ON);
    }
};
