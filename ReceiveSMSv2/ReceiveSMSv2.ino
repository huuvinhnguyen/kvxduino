#include <SoftwareSerial.h>

SoftwareSerial SIM900A(7, 8);
int ledPin = 13;


void setup()
{
  // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  SIM900A.begin(19200);
  // For serial monitor
  Serial.begin(19200);
  // Give time to your GSM shield log on to network
  delay(5000);

  // AT command to set SIM900 to SMS mode
  SIM900A.print("AT+CMGF=1\r");
  delay(100);
  // Set module to send SMS data to serial out upon receipt
  SIM900A.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  pinMode(ledPin, OUTPUT);

  

}

char buffer[250];
int pos = 0;

void resetBuffer()
{
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

char incoming_char = 0;

void loop()
{
  if (SIM900A.available() > 0) // Khong kiem tra dieu kien nay nhieu lan trong loop
  {
    
    char c = SIM900A.read();

    if (c == '\n')
    {
      Serial.println("Goi lenh");
      parseCommand(buffer);
      resetBuffer();
    }
    else
    {
      buffer[pos++] = c;
    }


  }

}

void parseCommand(char* buff)
{
    for(int i = 0; i < sizeof(buff); i++)
    {
        Serial.println(buff[i]);

        if ((buff[i] == 'B') && (buff[i+1] == 'A') && (buff[i+2] == 'T') )
        {
           digitalWrite(ledPin, HIGH);
            Serial.println("Bat den");
        }

        if ((buff[i] == 'T') && (buff[i+1] == 'A')  && (buff[i+2] == 'T'))
        {
           digitalWrite(ledPin, LOW);
            Serial.println("Tat den");
        }
        
    }

}
