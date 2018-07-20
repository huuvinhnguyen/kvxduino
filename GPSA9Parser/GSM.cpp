#include "GSM.h"
#include "Arduino.h"


bool GSM::isCallingFromSerial(SoftwareSerial *serial) {
  String number;

  while (serial->available()) {
    //    Serial.write(mySerial.read());

    char c = (char) serial->read();

    if (c == '\n') {


      if (number.indexOf("RING") != -1 ) {

//        serial->print("ATH \r \n");
        return true;
      }

      if (number.indexOf("+CLCC:") != -1) {
        //        Serial.println(number);
      }

      number = "";

    } else {


      number.concat(c);

    }

  }

  delay(1000);
  return false;
}
