#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySwitch.enableReceive(D2);

}

void loop() {
 

  if (mySwitch.available()) {
    //    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    //    mySwitch.resetAvailable();
    const char* rawBinany = dec2binWzerofill(mySwitch.getReceivedValue(), 30);
    
    char valueBinary[16];
    for (int i = 0 ; i < 15 ; i++) valueBinary[i] = rawBinany[i];
    valueBinary[15] = '\0';
    Serial.println(valueBinary);
    ///
    char idBinary[16];
    for (int i = 0 ; i < 15 ; i++) idBinary[i] = rawBinany[i+15];
    idBinary[15] = '\0';
    Serial.println(idBinary);
    Serial.println(rawBinany);
    mySwitch.resetAvailable();
    
  }
}

