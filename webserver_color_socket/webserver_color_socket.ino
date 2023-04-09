#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "ViewInteractor.h"
#include <WebSocketsServer.h>
#include <Adafruit_TCS34725.h>
#include <ColorName.hpp>
#include <FS.h>
#include <ESP8266mDNS.h>

#include <Servo.h>
Servo myservo;
int ser_pos_feeder = 70;
int ser_pos_fishtank = 120;
int pos = 0;



WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);
Adafruit_TCS34725 _colorSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

const int NUM_COLOR_NAMES = 12;
ColorName _colorNames[NUM_COLOR_NAMES] = {
  ColorName("Red", 150, 40, 40),
  ColorName("Red", 160, 50, 50),
  ColorName("Red", 255, 0, 0),
  ColorName("Orange", 160, 66, 34),
  ColorName("Yellow", 116, 93, 37),
  ColorName("Green", 0, 255, 0),
  ColorName("Green", 50, 130, 50),
  ColorName("Blue", 0, 0, 255),
  ColorName("Blue", 50, 50, 130),
  ColorName("Cyan", 0, 255, 255),
  ColorName("Magenta", 255, 0, 255),
  ColorName("Purple", 90, 80, 90)
};


void setupWifi() {
  delay(500);
  String host = String(ESP.getChipId());
  //  const char* host = "thietbi";
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  delay(500);

  //  const char* ssid = "khuonvienxanh";
  String ssid = "KVX" + String(ESP.getChipId());

  WiFi.softAP(ssid.c_str());
  IPAddress myIP = WiFi.softAPIP();

  long now = millis();

  while ((millis() - now) < 30000) {

    server.handleClient();

  }
}

void setup() {

      Serial.begin(115200);

  
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

  setupWifi();
  configureServer();
  if (!_colorSensor.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); 
  }

    myservo.attach(D4); 
    myservo.write(180);
    delay(1000);


}

void loop() {

    MDNS.update();

          // Read from color sensor
  _colorSensor.setInterrupt(false);      // turn on LED
//  delay(60);                    // takes 50ms to read
  _colorSensor.setInterrupt(true);       // turn off LED
    float sensedRed, sensedGreen, sensedBlue;

 
  _colorSensor.getRGB(&sensedRed, &sensedGreen, &sensedBlue);

  // Set the RGB LED color
  int rawRed = (int)sensedRed;
  int rawGreen = (int)sensedGreen;
  int rawBlue = (int)sensedBlue;

   // Get the name of closest color and print to serial
  ColorName *closestColorName = ColorName::getClosestColorName(_colorNames, 
    NUM_COLOR_NAMES, rawRed, rawGreen, rawBlue);
        String  colorName = closestColorName->getName();

      if (colorName.equals("Red")) {
              Serial.println("Red");
            activateServo();

        } else {
            activateServo2();
          }

    
//        String jsonString = "{";
//    jsonString += "\"red\":";
//    jsonString += rawRed;
//    jsonString += ",";
//    jsonString += "\"green\":";
//    jsonString += rawGreen;
//     jsonString += ",";
//       jsonString += "\"blue\":";
//    jsonString += rawBlue;
//
//    jsonString += "}";
//    webSocket.broadcastTXT(jsonString);
////    Serial.println(jsonString);
//
//    webSocket.loop();
//    server.handleClient();



}


void activateServo() {


  myservo.write(60);
  delay(500);
  myservo.write(180);
  delay(500);

}

void activateServo2() {


  myservo.write(180);
  delay(500);
  myservo.write(300);
  delay(500);

}

void configureServer() {

  ViewInteractor viewInteractor;
  viewInteractor.lookupFiles();

  server.onNotFound([]() {

    ViewInteractor viewInteractor;
    String path = server.uri();
    if (!viewInteractor.isFileRead(path))

      server.send(404, "text/plain", "FileNotFound");
    else {

      File file = viewInteractor.getFileRead(path);
      size_t sent = server.streamFile(file, viewInteractor.getContentType(path));
      file.close();
    }
  });

 

  server.on("/setting", []() {

    String serverString = server.arg("server");
    Serial.println(serverString);
    String username = server.arg("username");
    Serial.println(username);
    String password = server.arg("password");
    String port = server.arg("port");
    String topicpath = server.arg("topicpath");

    
    server.send(200, "text/html", "");

  });

    server.on("/", handleRoot);
    server.on("/color", handleColor);





  server.begin();
}

void handleRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/index.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

void handleColor() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/color.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      if (payload[0] == '#') {
        // we get RGB data

        // decode rgb data
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);


      }
      break;
  }
}
