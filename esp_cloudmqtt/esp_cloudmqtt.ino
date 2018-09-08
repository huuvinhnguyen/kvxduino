#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define TouchSensor 4 // Pin for capactitive touch sensor
boolean currentState = LOW;
boolean lastState = LOW;
boolean RelayState = LOW;
 
const char* ssid = "Ving";
const char* password =  "123456789";
const char* mqttServer = "m11.cloudmqtt.com";
const int mqttPort = 16242;
const char* mqttUser = "mntdttex";
const char* mqttPassword = "730VZPmi41Fd";
 
WiFiClient espClient;
PubSubClient client(espClient);
uint8_t ledPin = 2;
 
void setup() {
 
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(TouchSensor, INPUT);
 
  WiFi.begin(ssid, password);
 
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
  digitalWrite(ledPin,!digitalRead(ledPin));
 
}
 
void loop() {
  client.loop();

   currentState = digitalRead(TouchSensor);
    if (currentState == HIGH && lastState == LOW){
    Serial.println("pressed");
    delay(1);

    if (RelayState == HIGH){
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
}
