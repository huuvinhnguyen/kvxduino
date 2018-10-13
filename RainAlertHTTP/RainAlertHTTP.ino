const byte interruptPin = 13;
volatile boolean checkInterrupt = false;
#include <WiFiManager.h>
#include <ESP8266WiFi.h>

int numberOfInterrupts = 0;

unsigned long debounceTime = 1000;
unsigned long lastDebounce = 0;


void setup() {

  Serial.begin(115200);
  setupWiFi();

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

}

void handleInterrupt() {
  checkInterrupt = true;
}

void loop() {

  if (checkInterrupt == true && ( (millis() - lastDebounce)  > debounceTime )) {

    lastDebounce = millis();
    checkInterrupt = false;
    numberOfInterrupts++;

    Serial.print("Rain detected ");
    Serial.println(numberOfInterrupts);
    
    alertRain();


  } else checkInterrupt = false;


}

void setupWiFi() {

  delay(10000);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.startConfigPortal();
}

void alertRain() {
  delay(5000);

  Serial.print("connecting to ");
  const char* host = "maker.ifttt.com";
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/trigger/rain_alert/with/key/d9pfo19I0z65wZiQVhGgUO";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("Respond:");
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");  
}
