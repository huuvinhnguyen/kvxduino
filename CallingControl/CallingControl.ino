#include <SoftwareSerial.h>
#include "Coordinate.h"
#include "GSM.h"

byte RX = 10; // This is your RX-Pin on Arduino UNO,connect with A7 UTXD pin
byte TX = 11; // This is your TX-Pin on Arduino UNO,connect with a7 URXD pin
SoftwareSerial *A7board = new SoftwareSerial(RX, TX);

int legPin = 13;


void setup() {
  Serial.begin(115200);
  A7board->begin(115200);
  delay(200);
  pinMode(legPin, OUTPUT);
  

  delay(200);

  //  A7board->println("AT+CLIP=1 \r \n");
//  delay(5000);


}
//--------------------------------------------------------------------

void loop() {

  checkCalling();
      Serial.println("Test");

}

void checkCalling() {
  A7board->println("AT");
  digitalWrite(legPin, LOW);

  delay(3000);



  GSM gsm;
  bool isCalling = gsm.isCallingFromSerial(A7board);

  if (isCalling) {
    delay(1000);
    Serial.println("Calling");

    for (int i = 0; i < 2; i++) {
      
      digitalWrite(legPin, HIGH);
      delay(50);
      digitalWrite(legPin, LOW);
      delay(1000);
    }

//    A7board->print("ATH \r \n");

  }
}
