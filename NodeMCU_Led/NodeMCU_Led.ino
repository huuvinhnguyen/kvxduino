#include <ESP8266WiFi.h>

int ledPin = 13;
WiFiServer server(80);
const char* ssid = "Ving";
const char* password = "123456789";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  server.begin();
  Serial.println("Server started");

  Serial.print("Use this url to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  





}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH);
  delay(3000);
  digitalWrite(ledPin, LOW);
  delay(3000);


}
