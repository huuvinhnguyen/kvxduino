/* Arduino Tutorial - Watel Level Sensor 40mm
   More info: */

const int read = A0; //Sensor AO pin to Arduino pin A0
int value;          //Variable to store the incomming data

void setup()
{
  //Begin serial communication
  Serial.begin(9600);
  
}

void loop()
{
  value = analogRead(read); //Read data from analog pin and store it to value variable
  
  if (value<=480){ 
    Serial.println("Water level: 0mm - Empty!"); 
  }
  else if (value>480 && value<=530){ 
    Serial.println("Water level: 0mm to 5mm"); 
  }
  else if (value>530 && value<=615){ 
    Serial.println("Water level: 5mm to 10mm"); 
  }
  else if (value>615 && value<=660){ 
    Serial.println("Water level: 10mm to 15mm"); 
  } 
  else if (value>660 && value<=680){ 
    Serial.println("Water level: 15mm to 20mm"); 
  }
  else if (value>680 && value<=690){ 
    Serial.println("Water level: 20mm to 25mm"); 
  }
  else if (value>690 && value<=700){ 
    Serial.println("Water level: 25mm to 30mm"); 
  }
  else if (value>700 && value<=705){ 
    Serial.println("Water level: 30mm to 35mm"); 
  }
  else if (value>705){ 
    Serial.println("Water level: 35mm to 40mm"); 
  }
  
  delay(5000); // Check for new value every 5 sec
}
