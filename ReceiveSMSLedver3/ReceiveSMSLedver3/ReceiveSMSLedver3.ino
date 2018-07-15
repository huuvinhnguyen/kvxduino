
/*********
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial SIM900(7, 8);
//Variable to save incoming SMS characters
char incoming_char=0;
int ledPin = 13; 

void setup() {
  
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  SIM900.begin(115200);
  // For serial monitor
  Serial.begin(115200); 
  // Give time to your GSM shield log on to network
  delay(5000);

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);
  // Set module to send SMS data to serial out upon receipt 
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  pinMode(ledPin, OUTPUT);

}

String command;

void parseCommand(String com)
{
    com.toLowerCase();
   Serial.println(com);


  if(com.indexOf("bat") > -1) 
  {
    digitalWrite(ledPin, HIGH);
        Serial.println("bat den");

  }
  else if(com.indexOf("tat") > -1)
  {
    digitalWrite(ledPin, LOW);
        Serial.println("tat den");

  } else {
    Serial.println("Khong nhan dien");
  }
  
}

void loop() {

  
  // Display any text that the GSM shield sends out on the serial monitor
  if(SIM900.available() > 0) {
    delay(10);
    char c = (char)SIM900.read();

    if(c == '\n') 
    {
      Serial.println("Goi lenh");

      parseCommand(command);
      command = "";
    }
    else
    {
//      Serial.println(c);
      command.concat(c);

    }
    
  }
}
