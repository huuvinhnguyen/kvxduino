
#include <RCSwitch.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <dht.h>




RCSwitch mySwitch = RCSwitch();
int dhtPin    = 1;
dht DHT11;
#define DHT11PIN 4


int val = 0;

// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;

void setup() {

  //  Serial.begin(9600);
  //  pinMode(dhtPin, INPUT);


  mySwitch.enableTransmit(7);

  // Optional set protocol (default is 1, will work for most outlets)
  //  mySwitch.setProtocol(2);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);

  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
  setup_watchdog(9); // approximately 0.5 seconds sleep

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



  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag

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


    system_sleep();  // Send the unit to sleep

  }

}

// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() {

  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System actually sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out

  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON

}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);
  ww = bb;

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}


// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt = 1; // set global flag
}
