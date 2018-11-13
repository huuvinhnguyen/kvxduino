#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

ESP8266WebServer server(80);

#define TouchSensor 4 // Pin for capactitive touch sensor
boolean currentState = LOW;
boolean lastState = LOW;
boolean RelayState = LOW;

const char* ssid = "Ving";
const char* password =  "1234567890";
const char* mqttServer = "m15.cloudmqtt.com";
const int mqttPort = 11692;
const char* mqttUser = "quskfiwf";
const char* mqttPassword = "HKfqtBl47aBR";

WiFiClient espClient;
PubSubClient client(espClient);
uint8_t ledPin = 2;

void setup() {

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(TouchSensor, INPUT);
  setupWiFi();
//    WiFi.begin(ssid, password);
  
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

  client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("esp/test", 1);

  server.on("/vieweeprom", []() {


    Serial.println( "Read custom object from EEPROM: " );


    String content = "<html>";
    content += "<body>";
    content += "<h2>Your server:  <h1>" ;
    //    content += configuration.server ;
    content += "</h1>";
    //    content += configuration.user;
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";

    server.send(200, "text/html", content);
  });

  server.on("/setupwifi", []() {

    WiFi.begin(ssid, "123456789");


    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }


    server.send(200, "text/html", "Setup Wifi");
  });

  server.begin();

}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
  digitalWrite(ledPin, !digitalRead(ledPin));

}

void loop() {
  client.loop();

  currentState = digitalRead(TouchSensor);
  if (currentState == HIGH && lastState == LOW) {
    Serial.println("pressed");
    delay(1);

    if (RelayState == HIGH) {
      digitalWrite(ledPin, LOW);
      RelayState = LOW;
      client.publish("esp/test", "Hello from LOW");

    } else {
      digitalWrite(ledPin, HIGH);
      client.publish("esp/test", "Hello from HIGH");

      RelayState = HIGH;
    }
  }
  lastState = currentState;

  server.handleClient();
}

void setupWiFi() {

//  const char* ssid = "khuonvienxanh";
//
//  WiFi.softAP(ssid, "123456789");
//  IPAddress myIP = WiFi.softAPIP();
//  Serial.print("AP IP address: ");
//  Serial.println(myIP);

    delay(10000);
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(60);
    wifiManager.startConfigPortal();
  
//    const char* host = "khuonvienxanh";
//     MDNS.begin(host);
//       // Add service to MDNS
//    MDNS.addService("http", "tcp", 80);
//    MDNS.addService("ws", "tcp", 81);
}
