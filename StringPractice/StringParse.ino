

Coordinate coordinateFromString( String str) {

  String temp = "";
  float latitude;
  float longitude;
  float coordinateArray[2];
  Coordinate coordinate;

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

  coordinate.latitude = convertCoordinates(coordinateArray[0]);
  coordinate.longitude = convertCoordinates(coordinateArray[1]);

  return coordinate;
}

float convertCoordinates(float in_coords)
{
  //Initialize the location.
  float f = in_coords;
  // Get the first two digits by turning f into an integer, then doing an integer divide by 100;
  // firsttowdigits should be 77 at this point.
  int firsttwodigits = ((int)f) / 100;                             //This assumes that f < 10000.
  float nexttwodigits = f - (float)(firsttwodigits * 100);
  float theFinalAnswer = 0.000;
  theFinalAnswer = (float)(firsttwodigits + nexttwodigits / 60.000);
  return theFinalAnswer;
}
