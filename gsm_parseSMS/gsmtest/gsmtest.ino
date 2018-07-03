#include <SoftwareSerial.h>
SoftwareSerial GPRS(7, 8); // RX, TX

void setup()
{
  GPRS.begin(9600);
  Serial.begin(9600);
}

void loop()
{
  while(GPRS.available()) 
    Serial.write(GPRS.read());

  while (Serial.available()) 
      GPRS.write(Serial.read());
}
