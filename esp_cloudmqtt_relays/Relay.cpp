#include "Relay.h"

void Relay::setup(String mqttPath) {
  path = mqttPath;

  for (int i = 0; i < sizeof(relayPins); i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
}


void Relay::loop(callbackFunc func) {
  cb1 = func;

  long now = millis();

  if (now - startAttempedTime > longlast && isSetOnLastingActive) {
    if (startAttempedTime > 0) {
      isSetOnLastingActive = false;
    }

    startAttempedTime = now;
    setOn(true);
    Serial.println("Switch On");


  } else if (startAttempedTime != 0 && isSetOnLastingActive == false) {
    setOn(false);
    startAttempedTime = 0;
    Serial.println("Switch Off");
        cb1(201);
  }
}

void Relay::setOn(bool isOn) {
  uint8_t relayValue = (isOn) ? HIGH : LOW;
  for (int i = 0; i < sizeof(relayPins); i++) {
    digitalWrite(relayPins[i], relayValue);
    Serial.println("Switch value:");

    Serial.println(relayValue);

  }
  value = relayValue;

}


void Relay::switchOn() {
  isSetOnLastingActive = true;

}

void Relay::handleMessage(char *topic, String message) {
  String switchTopic = "switch";

  if (strcmp(topic + strlen(topic) - 6, "switch") == 0) {
  // Xử lý message
   }
 
  
  if (strcmp(topic, switchTopic.c_str()) == 0) {
    if (message.equals("1")) {

      setOn(true);

    } else if (message.equals("0")) {
      setOn(false);
    }
  }

  String switchon = "switchon";
  if (strcmp(topic, switchon.c_str()) == 0) {
    Serial.println("Switch ON: ");
    if (message == "done") {
      } else if (message.toInt() == 0) {
      switchOn();
    } else {
      setLonglast(message.toInt());
      switchOn();
    }
  }

  String longLastTopic = "longlast";
  if (strcmp(topic, longLastTopic.c_str()) == 0) {
    setLonglast(message.toInt());
    Serial.println("SetLonglast: ");
    Serial.println(message);

  }
}

void Relay::setLonglast(int seconds) {
  longlast = seconds * 1000;
}
