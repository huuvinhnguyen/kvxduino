#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include <ESP8266mDNS.h>
#include <RCSwitch.h>

ESP8266WebServer server(80);
RCSwitch mySwitch = RCSwitch();


struct Configuration {

  char mqttServer[30];
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
} configuration;

WiFiClient espClient;
PubSubClient client(espClient);
uint8_t ledPin = 2;

void setup() {

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  mySwitch.enableReceive(D2);


  setupWiFi();

  loadDataDefault();

  connectMQTT();

  configureServer();

}

void loop() {

  client.loop();
  server.handleClient();

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

    client.publish(idBinary, rawBinany);

    mySwitch.resetAvailable();
    
  }

}

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;

  Serial.print(str);
  if (str.equals("1") ) {
    digitalWrite(ledPin, LOW);

  } else {

    digitalWrite(ledPin, HIGH);
  }

  Serial.println();
  Serial.println("-----------------------");
  //  digitalWrite(ledPin, !digitalRead(ledPin));

}

void setupWiFi() {

  const char* host = "thietbi";
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);


  delay(10000);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(30);
  wifiManager.startConfigPortal();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }


  Serial.println("Connected to the WiFi network");
}

void loadDataDefault() {
  
  DataDefault<Configuration> dataDefault;
  delay(500);
  configuration = dataDefault.loadObject();
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

  server.on("/vieweeprom", []() {

    Serial.println( "Read custom object from EEPROM: " );

    String content = "<html>";
    content += "<body>";

    content += "<h1>User:  " ;
    content += configuration.mqttUser ;
    content += "</h1> <br>";

    content += "<h1>Password:  " ;
    content += configuration.mqttPassword ;
    content += "</h1> <br>";

    content += "<h1>Server:  " ;
    content += configuration.mqttServer ;
    content += "</h1> <br>";

    content += "<h1>Port:  " ;
    content += String(configuration.mqttPort) ;
    content += "</h1> <br>";
    content += "</body>";
    content += "</html>";

    server.send(200, "text/html", content);
  });

  server.on("/setting", []() {

    String serverString = server.arg("server");
    Serial.println(serverString);
    String username = server.arg("username");
    Serial.println(username);
    String password = server.arg("password");
    String port = server.arg("port");
    strcpy( configuration.mqttServer, serverString.c_str());
    strcpy( configuration.mqttUser, username.c_str());
    strcpy( configuration.mqttPassword, password.c_str());
    configuration.mqttPort = port.toInt();

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

    connectMQTT();
  });

  server.begin();
}

void connectMQTT() {

  client.setServer(configuration.mqttServer, configuration.mqttPort);
  client.setCallback(callback);

  int atempNum = 0;
  while (!client.connected() && (atempNum < 10)) {
    
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", configuration.mqttUser, configuration.mqttPassword )) {

      Serial.println("connected");
      client.subscribe("switch", 1);
      client.publish("FirstPing", "Hello ");
    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      atempNum++;
    }
  }
}

char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64]; 
  unsigned int i=0;

  while (Dec > 0) {
    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';
  
  return bin;
}
