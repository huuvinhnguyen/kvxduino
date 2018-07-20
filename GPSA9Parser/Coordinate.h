#include "Arduino.h"

typedef struct Coordinate { 
    float latitude = 0; 
    float longitude = 0;
} Coordinate;

class GPS  {
  public:
  Coordinate coordinateFromString(String str);
  Coordinate *getCoordinates();
  
  private:
  bool isValid(String gpsString);
  float convertCoordinates(float in_coords);
};
