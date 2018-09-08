#include <SoftwareSerial.h>
#include "Coordinate.h"
#include "GSM.h"

byte RX = 8; // This is your RX-Pin on Arduino UNO,connect with A7 UTXD pin
byte TX = 9; // This is your TX-Pin on Arduino UNO,connect with a7 URXD pin
SoftwareSerial *A7board = new SoftwareSerial(RX, TX);

String gpsString = "";


void sendSMS() {



  Serial.println("Sending sms");

  for ( int i = 0; i < 20; ++i) {
    A7board->println("AT+GPS=1");
    delay(1000);
    A7board->println("AT+LOCATION=2");
    delay(1000);
    GPS gps;

    while (A7board->available()) {
      //    Serial.write(A7board->read());

      char c = (char)A7board->read();
      if (c == '\n') {

        Coordinate coordinate = gps.coordinateFromString(gpsString);
        Serial.println(gpsString);
        if (coordinate.latitude != 0 && coordinate.longitude != 0) {


          char latString[20];
          char longString[20];
          dtostrf(coordinate.latitude, 5, 5, latString);

          dtostrf(coordinate.longitude, 5, 5, longString);

          String message =  "http://google.com/maps/place/";
          message += latString;


          message += ",";


          message += longString;

          Serial.println(message);
          A7board->println("AT+GPS=0");
          delay(1000);

        }

        gpsString = "";
      } else {
        gpsString.concat(c);

      }

    }
    delay(2000);
  }
}

void getCoordinates() {
  int num = 3;
  Coordinate *coordinates = getCoordinates(num);

  String content = "";

  for (int i = 0; i < num; i ++) {
    Coordinate coordinate = coordinates[i];
   
    char latString[20];
    char longString[20];
    dtostrf(coordinate.latitude, 5, 5, latString);

    dtostrf(coordinate.longitude, 5, 5, longString);

    String message =  "http://google.com/maps/place/";
    message += latString;


    message += ",";


    message += longString;
    message += "\n";

    content += message;

  }

  GSM gsm;
  gsm.sendMessage(content, A7board);
  

  Serial.println(content);
}

void setup() {
  Serial.begin(9600);
  A7board->begin(9600);
  pinMode(13, OUTPUT);
  delay(200);

  Serial.println("Send AT command");
  A7board->println("AT");
  delay(5000);

  Serial.println((char)26);


}
//--------------------------------------------------------------------

void loop() {
  checkCalling();
  if (Serial.available()) {
    char c = (char)Serial.read();
    switch (c) {
      case 's':
        sendSMS();

        break;
      case 'r': break;

      case 'g':
        getCoordinates();
        break;
      case 'c':
        checkCalling();
        break;
    }
  }
 
}

void checkCalling() {

  GSM gsm;
  bool isCalling = gsm.isCallingFromSerial(A7board);

  if (isCalling) {
    Serial.println("Calling");

    digitalWrite(13, HIGH);       
    delay(1000);                  
    digitalWrite(13, LOW);        
    delay(1000);
    getCoordinates();

  }

  A7board->print("ATH \r \n");

}


Coordinate* getCoordinates(int num) {
    A7board->println("AT+GPS=1");
  delay(1000);

  String gpsString2 = "";
  Coordinate* coordinates = new Coordinate[num];
  int i = 0;
  while (i < num) {

    A7board->println("AT+LOCATION=2");
    delay(1000);
    GPS gps;

    while (A7board->available()) {
      //    Serial.write(A7board->read());

      char c = (char)A7board->read();
      if (c == '\n') {

        Coordinate coordinate = gps.coordinateFromString(gpsString2);
        Serial.println(gpsString2);
        if (coordinate.latitude != 0 && coordinate.longitude != 0) {

          coordinates[i] = coordinate;
          i++;

          char latString[20];
          char longString[20];
          dtostrf(coordinate.latitude, 5, 5, latString);

          dtostrf(coordinate.longitude, 5, 5, longString);

          String message =  "";
          message += latString;


          message += ",";


          message += longString;


          Serial.println(message);

        }

        gpsString2 = "";
      } else {

        gpsString2.concat(c);
      }
    }
  }
  
  A7board->println("AT+GPS=0");
  delay(1000);
  return coordinates;

}
