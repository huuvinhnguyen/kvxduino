#include <WiFiManager.h>
#include <ESP8266WiFi.h>

int pirPin = D7;
int val;

void setup()
{
  Serial.begin(115200);
  setupWiFi();

}

void loop()
{
  val = digitalRead(pirPin);
  //low = no motion, high = motion
  if (val == LOW)
  {
    Serial.println("No motion");
  }
  else
  {
    Serial.println("Motion detected  ALARM");
    alertMotion();
  }

  delay(1000);
}

void alertMotion() {
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
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void setupWiFi() {

  delay(10000);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.startConfigPortal();
}
