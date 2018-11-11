#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "DataDefault.h"
#include "ViewInteractor.h"

ESP8266WebServer server(80);

struct Configuration {
  
  char server[50];
  char user[50] ;
} configuration;


void setup() {

  Serial.begin(115200);
  setupWiFi();

  delay(5000);


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
    strcpy( configuration.server, serverString.c_str());
    strcpy( configuration.user, username.c_str());
    DataDefault<Configuration> dataDefault;
    dataDefault.saveObject(configuration);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your file Id:  <h1>" ;
    //      content += fileId ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);
  });

  server.on("/vieweeprom", []() {

    DataDefault<Configuration> dataDefault;
    Configuration configuration = dataDefault.loadObject();

    Serial.println( "Read custom object from EEPROM: " );
    Serial.println( configuration.server );
    Serial.println( configuration.user);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your server:  <h1>" ;
    content += configuration.server ;
    content += "</h1>";
    content += configuration.user;
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";

    server.send(200, "text/html", content);
  });

  server.begin();

}


void loop() {
  
  server.handleClient();
}


void setupWiFi() {

  const char* ssid = "khuonvienxanh";

  WiFi.softAP(ssid, "123456789");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

