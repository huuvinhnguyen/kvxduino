#include <SoftwareSerial.h>

byte RX = 10; // This is your RX-Pin on Arduino UNO,connect with A7 UTXD pin
byte TX = 11; // This is your TX-Pin on Arduino UNO,connect with a7 URXD pin
SoftwareSerial *A7board = new SoftwareSerial(RX, TX); 

void print_result()
{
  Serial.print("A7 board info: ");
  while( A7board->available() != 0)
    Serial.write( A7board->read());
  Serial.println();  
}
//--------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  A7board->begin(115200);
  delay(200); 

  Serial.println("Send AT command");  
  A7board->println("AT");
  delay(25000);
  print_result();
  
  Serial.println("AT+GPS turn on");  
  A7board->println("AT+GPS=1");  
  delay(10000);
  print_result();
  
  Serial.println("AT+GPSRD turn on");  
  A7board->println("AT+GPSRD=1");  
  delay(10000);
  print_result();

}
//--------------------------------------------------------------------

void loop() {
  print_result();
  delay(2000);
}
//--------------------------------------------------------------------
