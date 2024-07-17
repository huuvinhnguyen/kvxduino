#include <EEPROM.h>
#include <WiFi.h>

template <class T>
class DataDefault {

  public:
  
    String stringFromEEPROM() {
      String string = "";
      
      for (int i = 0; i < EEPROM.length(); ++i) {

        string += char(EEPROM.read(i));
      }

      return string;
    }
    
    void saveEEPROM(String content) {
      for (int i = 0; i < 96; ++i) {

        EEPROM.write(i, 0);
      }

      for (int i = 0; i < content.length(); ++i) {
        EEPROM.write(i, content[i]);
        Serial.print("Wrote: ");
        Serial.println(content[i]);
      }

      EEPROM.commit();
    }
    
    void saveObject (T t) {
      
      EEPROM.begin(512);
      EEPROM.put(0, t);
      EEPROM.commit();
    }

    T loadObject() {
      
      EEPROM.begin(512);
      T t;
      EEPROM.get( 0, t );
      return t;
    }
};
