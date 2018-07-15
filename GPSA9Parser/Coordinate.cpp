#include "Coordinate.h"

bool GPS::isValid(String gpsString) {
  int comasNum = 0;
  int dotNum = 0;

  for (int i = 0; i < gpsString.length(); i++) {

    char c = gpsString[i];
    if (c == ',') comasNum++;
    if (c == '.') dotNum++;

  }

  if (comasNum != 1) return false;
  if (dotNum != 2) return false;

  return true;
}

Coordinate* GPS::getCoordinates() {

  Coordinate* coordinates = new Coordinate[3];
  return coordinates;

}


float GPS::convertCoordinates(float in_coords) {

  float f = in_coords;
  // Get the first two digits by turning f into an integer, then doing an integer divide by 100;
  // firsttowdigits should be 77 at this point.
  int firsttwodigits = ((int)f) / 100;                             //This assumes that f < 10000.
  float nexttwodigits = f - (float)(firsttwodigits * 100);
  float theFinalAnswer = 0.000;
  theFinalAnswer = (float)(firsttwodigits + nexttwodigits / 60.000);
  return theFinalAnswer;
}


Coordinate GPS::coordinateFromString( String str) {

  String temp = "";
  float latitude = 0;
  float longitude = 0;
  float coordinateArray[2];
  Coordinate coordinate;

  GPS gps;

  if (!gps.isValid(str)) return coordinate;

  int pos = 0;

  for (int i = 0; i < str.length(); i++ ) {


    if (str[i] == ',' || i == str.length() - 1) {

      coordinateArray[pos] = temp.toFloat();
      temp = "";
      pos++;

    } else {

      temp.concat(str[i]);
    }
  }

  coordinate.latitude = gps.convertCoordinates(coordinateArray[0]);
  coordinate.longitude = gps.convertCoordinates(coordinateArray[1]);

  return coordinate;
}
