#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Arduino.h"
#include "Coordinate.h"


String serialResponse = "";
char sz[] = "Here; is some; sample;100;data;1.414;1020";

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(5);
}


void loop()
{

  if ( Serial.available()) {
    //    serialResponse = Serial.readStringUntil('\r\n');
    //    char n = (char)Serial.read();
    //    num(n);
    //    // Convert from String Object to String.
    //    char buf[sizeof(sz)];
    //    serialResponse.toCharArray(buf, sizeof(buf));
    //    char *p = buf;
    //    char *str;
    //    while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
    //      Serial.println(str);
    //
    //      //Check for numberic
    //
    //  }

    char n = (char)Serial.read();
    if (n != '\r')  isNum(n);
    //
    //  stringEmer("1047.3851,10640.5012 ");
    //  stringArrayAppend();
    //    conv_coords(10640.5012);

    Coordinate coord = coordinateFromString("1047.3851,10640.5012 ");
    Serial.print("Coordinate: ");
    Serial.print(coord.latitude, 4);
    Serial.print(",");
    Serial.print(coord.longitude, 4);
    Serial.println();


    //    struct angles theAngles;

  }
}



void stringEmer(String str) {
  Serial.println("FFFFFF");
  for (int i = 0; i < str.length(); i++) {
    if (isNum(str[i])) Serial.println(str[i]);

  }
}

boolean isNum(char c) {

  if (isDigit(c))  return true;
  return false;
}

void stringArrayAppend() {
  String text = "1047.3851,10640.5012 ";
  String temp = "";
  String strAr[4];
  int pos = 0;

  for (int i = 0; i < text.length(); i++ ) {
    if (isDigit(text[i])) {
      temp.concat(text[i]);

    } else {

      strAr[pos] = temp;
      pos++;
      temp = "";
    }

    if (i == text.length() - 1) {

      strAr[pos] = temp;
      temp = "";
    }

  }

  for (int i = 0; i < 4; i++) {
    Serial.println(strAr[i]);

  }


}

//int parse()
/// //{
//    char string[] = "AT+CMGL=\"ALL\"\n"
//    "\n"
//    "+CMGL: 2,\"REC READ\",\"+639321576684\",\"Ralph Manzano\",\"16/04/25,21:51:33+32\"\n"
//    "Yow!!!\n"
//    "\n"
//    "OK\n";
//    char delimiter[] = ",\n";
//
//    // initialize first part (string, delimiter)
//    char* ptr = strtok(string, delimiter);
//
//    while(ptr != NULL) {
//        printf("found one part: %s\n", ptr);
//        // create next part
//        ptr = strtok(NULL, delimiter);
//    }
//
//    return 1;
// }

//1047.3851,10640.5012 
//ddmm.
