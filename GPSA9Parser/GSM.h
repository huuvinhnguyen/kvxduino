#include <SoftwareSerial.h>

class GSM {
  public:
   bool isCallingFromSerial(SoftwareSerial *serial); 
   void GSM::sendMessage(String str, SoftwareSerial *serial); 
};
