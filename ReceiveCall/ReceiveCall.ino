
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
  mySerial.print("AT+ATA\r\n");

 
 
}

void loop()
{
     
    while(mySerial.available() > 0)
    {


      digitalWrite(13, HIGH);       // sets the digital pin 13 on
      delay(10);                  // waits for a second
      digitalWrite(13, LOW);        // sets the digital pin 13 off
      delay(10);
      
//       Serial.println("MySerial1");
        char c = mySerial.read();
       Serial.print(c);

       mySerial.println("ATH");


     }   
}
