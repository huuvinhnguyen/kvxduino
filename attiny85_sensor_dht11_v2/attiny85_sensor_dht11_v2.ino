
#include <RCSwitch.h>
#include <dht.h>


#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer


RCSwitch mySwitch = RCSwitch();
int dhtPin    = 1;
dht DHT11;
#define DHT11PIN 4

//Pin definitions for regular Arduino Uno (used during development)
/*const byte buzzer1 = 8;
 const byte buzzer2 = 9;
 const byte statLED = 10;
 const byte waterSensor = A0;*/

//Pin definitions for ATtiny

const byte waterSensor = A1;


//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  //Don't do anything. This is just here so that we wake up.
}

void setup()
{

    mySwitch.enableTransmit(7);

  
  //pinMode(waterSensor, INPUT_PULLUP);
  pinMode(2, INPUT); //When setting the pin mode we have to use 2 instead of A1
  digitalWrite(2, HIGH); //Hack for getting around INPUT_PULLUP



  delay(100);


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


void loop() 
{
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  setup_watchdog(9 ); //Setup watchdog to go off after 1sec
  sleep_mode(); //Go to sleep! Wake up 1sec later and check water

  //Check for water
  ADCSRA |= (1<<ADEN); //Enable ADC

  ////*****/////
  wdt_disable(); //Turn off the WDT!!

  int chk = DHT11.read11(DHT11PIN);
    float hu = DHT11.humidity * 100;
    float te = DHT11.temperature * 100;

    char *strTemp = dec2binWzerofill(te, 16);

    char teId[16] = "100000000000001";
    teId[15] = '\0';
    char sendingTe[32];
    sprintf(sendingTe, "%s%s", strTemp, teId);
    mySwitch.send(sendingTe);


    char *strHu = dec2binWzerofill(hu, 16);
    char huId[16] = "110000000000001";
    huId[15] = '\0';
    char sendingHu[32];
    sprintf(sendingHu, "%s%s", strHu, huId);
    mySwitch.send(sendingHu);
  

}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}
