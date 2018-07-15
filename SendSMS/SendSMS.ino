#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);
void setup()
{
  mySerial.begin(115200);   // Setting the baud rate of GSM Module
  Serial.begin(115200);    // Setting the baud rate of Serial Monitor (Arduino)
  delay(100);
}


void loop()
{
  if (Serial.available() > 0)
    switch (Serial.read())
    {
      case 's':
        SendMessage();
        break;
      case 'r':
        RecieveMessage();
        break;
    }

  if (mySerial.available() > 0)
  {
    Serial.write(mySerial.read());
  }
}

void SendMessage()
{
  Serial.print("Sending message");
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+84988249454\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println("http://google.com/maps/place/49.46800006494457,17.11514008755796");// The SMS text you want to send
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
void RecieveMessage()
{
  delay(1000);
}
