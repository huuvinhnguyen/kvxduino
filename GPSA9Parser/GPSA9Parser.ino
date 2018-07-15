#include <SoftwareSerial.h>
#include "Coordinate.h"

byte RX = 10; // This is your RX-Pin on Arduino UNO,connect with A7 UTXD pin
byte TX = 11; // This is your TX-Pin on Arduino UNO,connect with a7 URXD pin
SoftwareSerial *A7board = new SoftwareSerial(RX, TX);

String gpsString = "";

int num = 0;

void print_result()
{
  Serial.print("board info: ");
  while ( A7board->available())
  {
    char c = (char)A7board->read();
    GPS gps;
    if (c == '\n') {

      Coordinate coordinate = gps.coordinateFromString(gpsString);
      Serial.println(gpsString);
      if (coordinate.latitude > 0 && coordinate.longitude > 0) {

        Serial.print("Coordinate: ");
        Serial.print(coordinate.latitude, 4);
        Serial.print(",");
        Serial.print(coordinate.longitude, 4);
        Serial.println();

      }

      gpsString = "";
    } else {
      gpsString.concat(c);

    }

  }

  Serial.println();
}
//--------------------------------------------------------------------

void sendSMS() {



  Serial.println("Sending sms");

  for ( int i = 0; i < 20; ++i) {

    A7board->println("AT+LOCATION=2");
    delay(2000);
    GPS gps;

    while (A7board->available()) {
      //    Serial.write(A7board->read());

      char c = (char)A7board->read();
      if (c == '\n') {

        Coordinate coordinate = gps.coordinateFromString(gpsString);
        Serial.println(gpsString);
        if (coordinate.latitude != 0 && coordinate.longitude != 0) {

          num++;

          char latString[20];
          char longString[20];
          dtostrf(coordinate.latitude, 5, 5, latString); 

          dtostrf(coordinate.longitude, 5, 5, longString); 

          String message =  "http://google.com/maps/place/";
          message +=latString;


          message +=",";


          message += longString;


          Serial.println(message);


        }

        gpsString = "";
      } else {
        gpsString.concat(c);

      }

    }
    delay(2000);
  }

}

void setup() {
  Serial.begin(115200);
  A7board->begin(115200);
  delay(200);

  Serial.println("Send AT command");
  A7board->println("AT");
  delay(5000);
  print_result();

  Serial.println("AT+GPS turn on");
  Serial.println((char)26);
  //  A7board->println("AT+GIZTRACKER=1,1,1,1,1");
  //  A7board->println("AT+GPSUPGRADE=on");
  A7board->println("AT+GPS=1");
  delay(10000);
  print_result();

  Serial.println("AT+GPSRD turn on");
  //  A7board->println((char)26);
  Serial.println((char)26);
  //  A7board->println("AT+GPSRD=1");
  delay(10000);
  print_result();

  //
  //      A7board->println("AT+LOCATION=2");
  //       delay(20000);




  Serial.println((char)26);

}
//--------------------------------------------------------------------

void loop() {

  if (Serial.available()) {
    char c = (char)Serial.read();
    switch (c) {
      case 's':
        sendSMS();

        break;
      case 'r': break;
    }
  }
  //    A7board->println("AT+LOCATION=1");
  //    A7board->println("AT+GPS=1");
  //  Serial.println((char)26);



  //   A7board->println("AT+LOCATION=2");

  //    A7board->println("AT+GPS=1");


  //  print_result();
  //  delay(2000);
}

void sendMessage(String str)
{
  A7board->println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  A7board->println("AT+CMGS=\"+84988249454\"\r"); // Replace x with mobile number
  delay(1000);
  A7board->println(str);// The SMS text you want to send
  delay(100);
  A7board->println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

Coordinate* getCoordinates(int num) {

  Coordinate* coordinates = new Coordinate[num];
  return coordinates;
  
}
