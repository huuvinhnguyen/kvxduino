
//Serial Relay - pour Arduino 1.0
//
// Arduino will patch a serial link between the computer
// and the GPRS Shield at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART

#include <SoftwareSerial.h>    // For Arduino 1.0

SoftwareSerial mySerial(10, 11); // For Arduino 1.0

boolean flag;

void setup()
{
  flag = false;
  mySerial.begin(115200);               // the GPRS baud rate
  Serial.begin(115200);                 // the GPRS baud rate
  pinMode(13, OUTPUT);
  delay(1000);

}


String str;
void loop()
{

  if (mySerial.available() > 0)
  {

    char c = (char) mySerial.read();

    if (c == '\n') {

      

      //      Serial.println(str);

//      mySerial.print("AT+CLIP=1 \r \n");



      readSerial();
      //         Serial.write(mySerial.read());
      //
      str = "";
    } else {

      str.concat(c);
    }


  }
}

  String number;

void readSerial() {

  while (mySerial.available()) {
    //    Serial.write(mySerial.read());

    char c = (char) mySerial.read();

    if (c == '\n') {


      if (number.indexOf("RING") != -1 ) {

        Serial.println("Dang goi");
        mySerial.print("ATH \r \n");
        
      }


      Serial.println(number);

      if (number.indexOf("+CLCC:") != -1) {
//        Serial.println(number);
      }

      number = "";

    } else {


      number.concat(c);

    }

  }

  delay(1000);


}
