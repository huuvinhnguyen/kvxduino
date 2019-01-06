//Onewire sensor library
#include <OneWire.h>
#include <DallasTemperature.h>
//I2C OLED SSD1306 screen library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ONE_WIRE_BUS    2               // Data wire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);          // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature.

#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
  // start serial port
  Serial.begin(115200);
  Serial.println("------------------------------------------");
  Serial.println("Example for VBLUno51 board");
  Serial.println("Dallas Temperature IC Control Library Demo");

  //start oled
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("VBLUno51 board:");
  oled.display();
    
  // Start up the library
  sensors.begin();
  delay(1000);
}

/*
 * Main function, get and show the temperature
 */
 double temp = 0.0f;
void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  temp = sensors.getTempCByIndex(0);
  Serial.println(temp);  
  
  oled.clearDisplay();  
  oled.setCursor(0,0);
  oled.println("Temperature (C):\r\n");
  oled.println(temp);
  oled.display();
  
  delay(1000);
}
