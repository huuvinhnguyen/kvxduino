#include <CayenneMQTTESP8266.h>
#include <DHT.h>
#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial

char ssid[] = "Ving";
char password[] = "123456789";

char username[] = "b5e1a930-8bfa-11e8-b90f-f7e25f9cf9c8";
char mqtt_password[] = "03a8dc04319ce456b606c345d8508e6509a864ab";
char client_id[] = "29520c10-8cf1-11e8-8fba-979723b0b5e0";

DHT dht(D2, DHT11);

void setup() {
  // put your setup code here, to run once:
  Cayenne.begin(username, mqtt_password, client_id, ssid, password);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:
  Cayenne.loop();
  float temp = dht.readTemperature(true);
  float hum = dht.readHumidity();
  Cayenne.virtualWrite(1, temp, TYPE_TEMPERATURE, UNIT_FAHRENHEIT);
  Cayenne.virtualWrite(2, hum, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);


}

CAYENNE_IN(0)
{
  digitalWrite(2, !getValue.asInt());
}
