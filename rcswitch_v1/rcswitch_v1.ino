/* 
Example for different sending methods
http://code.google.com/p/rc-switch/ 
*/ 
#include <rcswitch.h>
RCSwitch mySwitch = RCSwitch();
 
void setup() {
   // Transmitter is connected to Attiny Pin PB3  <--
   // That is physical pin2
  mySwitch.enableTransmit(3);
 
  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
}
 
void loop() {
  /* See Example: TypeA_WithDIPSwitches */
  mySwitch.switchOn("10101", "10000");
  delay(1000);
  mySwitch.switchOff("10101", "10000");
  delay(2000);
 }
