#include "GSM.h"
#include "Arduino.h"


bool GSM::isCallingFromSerial(SoftwareSerial *serial) {
  String number = "";

  while (serial->available()) {

    char c = (char) serial->read();

    if (c == '\n') {

      Serial.println(number);

      if (number.indexOf("RING") != -1 ) {

        Serial.println(number);
        serial->print("ATH \r \n");
        return true;
      }

      number = "";

    } else {


      number.concat(c);

    }

  }

  delay(100);
  return false;
}
