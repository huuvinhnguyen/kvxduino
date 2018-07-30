#include <SoftwareSerial.h>
#include "Coordinate.h"
#include "GSM.h"

byte RX = 10; // This is your RX-Pin on Arduino UNO,connect with A7 UTXD pin
byte TX = 11; // This is your TX-Pin on Arduino UNO,connect with a7 URXD pin
SoftwareSerial *A7board = new SoftwareSerial(RX, TX);

String gpsString = "";


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


          char latString[20];
          char longString[20];
          dtostrf(coordinate.latitude, 5, 5, latString);

          dtostrf(coordinate.longitude, 5, 5, longString);

          String message =  "http://google.com/maps/place/";
          message += latString;


          message += ",";


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

void getCoordinates() {
  int num = 8;
  Coordinate *coordinates = getCoordinates(num);

  String content = "";

  for (int i = 0; i < num; i ++) {
    Coordinate coordinate = coordinates[i];
    //    Serial.print("Coordinate: ");
    //    Serial.print(coordinate.latitude, 5);
    //    Serial.print(",");
    //    Serial.print(coordinate.longitude, 5);
    //    Serial.println();

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
    GSM gsm;
    gsm.sendMessage(content, A7board);

//    sendMessage(content);

  }

  Serial.println(content);
}

void setup() {
  Serial.begin(115200);
  A7board->begin(115200);
  delay(200);

  Serial.println("Send AT command");
  A7board->println("AT");
  delay(5000);

  Serial.println("AT+GPS turn on");
  Serial.println((char)26);
  //  A7board->println("AT+GIZTRACKER=1,1,1,1,1");
  //  A7board->println("AT+GPSUPGRADE=on");
  A7board->println("AT+GPS=1");
  delay(10000);
  //  print_result();

  Serial.println("AT+GPSRD turn on");
  //  A7board->println((char)26);
  Serial.println((char)26);
  //  A7board->println("AT+GPSRD=1");
  //  delay(10000);
  //  print_result();

  //
  //      A7board->println("AT+LOCATION=2");
  //       delay(20000);




  Serial.println((char)26);

}
//--------------------------------------------------------------------

void loop() {
//  checkCalling();
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
  //    A7board->println("AT+LOCATION=1");
  //    A7board->println("AT+GPS=1");
  //  Serial.println((char)26);



  //   A7board->println("AT+LOCATION=2");

  //    A7board->println("AT+GPS=1");


  //  print_result();
  //  delay(2000);
}

void checkCalling() {

  GSM gsm;
  bool isCalling = gsm.isCallingFromSerial(A7board);

  if (isCalling) {
    Serial.println("Calling");
  }

    A7board->print("ATH \r \n");

}


Coordinate* getCoordinates(int num) {

  Coordinate* coordinates = new Coordinate[num];
  int i = 0;
  while (i < num) {

    A7board->println("AT+LOCATION=2");
    delay(5000);
    GPS gps;

    while (A7board->available()) {
      //    Serial.write(A7board->read());

      char c = (char)A7board->read();
      if (c == '\n') {

        Coordinate coordinate = gps.coordinateFromString(gpsString);
        Serial.println(gpsString);
        if (coordinate.latitude != 0 && coordinate.longitude != 0) {

          coordinates[i] = coordinate;
          i++;

          char latString[20];
          char longString[20];
          dtostrf(coordinate.latitude, 5, 5, latString);

          dtostrf(coordinate.longitude, 5, 5, longString);

          String message =  "http://google.com/maps/place/";
          message += latString;


          message += ",";


          message += longString;


          Serial.println(message);

        }

        gpsString = "";
      } else {

        gpsString.concat(c);
      }
    }
  }
  delay(5000);
  return coordinates;

}
