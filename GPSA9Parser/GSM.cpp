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

void GSM::sendMessage(String str, SoftwareSerial *serial)
{
  serial->println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  serial->println("AT+CMGS=\"+84988249454\"\r"); 
  delay(1000);
  serial->println(str);// The SMS text you want to send
  delay(100);
  serial->println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
