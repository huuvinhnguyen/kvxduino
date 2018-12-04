#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySwitch.enableReceive(D2); 

}

void loop() {
//  // put your main code here, to run repeatedly:
//    if (mySwitch.available()) {
//    
//    Serial.print("Received ");
//    Serial.print( mySwitch.getReceivedValue() );
//    Serial.print(" / ");
//    Serial.print( mySwitch.getReceivedBitlength() );
//    Serial.print("bit ");
//    Serial.print("Protocol: ");
//    Serial.println( mySwitch.getReceivedProtocol() );
//    Serial.println("Raw: ");
//
//    mySwitch.resetAvailable();
//  }


  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
