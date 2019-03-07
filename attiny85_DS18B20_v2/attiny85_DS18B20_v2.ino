///*********
//  Rui Santos
//  Complete project details at http://randomnerdtutorials.com
//  Based on the Dallas Temperature Library example
//*********/
//
//#include <OneWire.h>
//#include <DallasTemperature.h>
//
//// Data wire is conntec to the Arduino digital pin 2
//#define ONE_WIRE_BUS 2
//
//// Setup a oneWire instance to communicate with any OneWire devices
//OneWire oneWire(ONE_WIRE_BUS);
//
//// Pass our oneWire reference to Dallas Temperature sensor
//DallasTemperature sensors(&oneWire);
//
//void setup(void)
//{
//
//  // Start up the library
//  sensors.begin();
//}
//
//
//void loop(void){
//  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
//  sensors.requestTemperatures();
//
//  int temp = sensors.getTempCByIndex(0) * 100
//  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
////  Serial.print(sensors.getTempCByIndex(0));
////  Serial.print(" - Fahrenheit temperature: ");
////  Serial.println(sensors.getTempFByIndex(0));
//  delay(1000);
//}


#include <RCSwitch.h>
#include <avr/sleep.h>
#include <avr/wdt.h> //Needed to enable/disable watch dog timer
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


RCSwitch mySwitch = RCSwitch();
//int pirPin    = 1;
int val = 0;
int watchdog_counter = 2;

// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;

void setup() {

  //  Serial.begin(9600);
  //  pinMode(pirPin, INPUT);

  mySwitch.enableTransmit(7);

  sensors.begin();

  // Optional set protocol (default is 1, will work for most outlets)
  //  mySwitch.setProtocol(2);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);

  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);

  //Power down various bits of hardware to lower power usage
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();

}

static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0) {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}

void loop() {

  ADCSRA &= ~(1 << ADEN); //Disable ADC, saves ~230uA
  setup_watchdog(9 ); //Setup watchdog to go off after 1sec
  sleep_mode(); //Go to sleep! Wake up 1sec later and check water

  //Check for water
  ADCSRA |= (1 << ADEN); //Enable ADC

  ////*****/////
  wdt_disable(); //Turn off the WDT!!

  if (watchdog_counter > 1)
  {

    watchdog_counter = 0;

    sensors.requestTemperatures() ;

    float te = sensors.getTempCByIndex(0) * 100;

    char *strTemp = dec2binWzerofill(te, 16);


    char teId[16] = "100000000000011";

    teId[15] = '\0';
    char sendingTe[32];
    sprintf(sendingTe, "%s%s", strTemp, teId);

    mySwitch.send(sendingTe);

  }


}




//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7;
  if (timerPrescaler > 7) bb |= (1 << 5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1 << WDRF); //Clear the watch dog reset
  WDTCR |= (1 << WDCE) | (1 << WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {

  watchdog_counter++;
}
