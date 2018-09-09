#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

String handleRoot() {
  String content = "";
  content += "<html>";
  content += "<body>";
  content += "<h2>HTML Forms</h2>";

  content += "<form action=\"setting\">";
  content += "ID:";
  content += "<input type=\"text\" name=\"fileId\" >";
  content += "<input type=\"submit\" value=\"Submit\">";
  content += "</form>";
  content += "</body>";
  content += "</html>";
  return content;
}

void setup() {

  Serial.begin(115200);
  setupWiFi();

  EEPROM.begin(512);

  saveEEPROM("Hello world!");

  delay(5000);

  server.on("/", []() {
    server.send(200, "text/html", handleRoot());
  });

  server.on("/setting", []() {
    
    String fileId = server.arg("fileId");
    saveEEPROM(fileId);
    String content = "<html>";
    content += "<body>";
    content += "<h2>Your file Id:  <h1>" ;
    content += fileId ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);
  });

  server.on("/vieweeprom", []() {
    
    String content = "";
    for (int i = 0; i < EEPROM.length(); ++i) {
      
      content += char(EEPROM.read(i));
    }

    server.send(200, "text/html", content.c_str());
  });

  server.begin();

}

int incomingByte = 0;

void loop() {

  if (Serial.available() > 0) {

    incomingByte = Serial.read();

    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }

  printEEPROM();

  server.handleClient();
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

void printEEPROM() {
  String content = "";
  for (int i = 0; i < EEPROM.length(); ++i) {
    content += char(EEPROM.read(i));
  }
  Serial.print("EEPROM content: ") ;
  Serial.println(content);
  delay(3000);

}


const char* ssid = "khuonvienxanh";
void setupWiFi() {

  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}
