#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ViewInteractor.h"
#include "DataDefault.h"
#include <ESP8266mDNS.h>
#include <RCSwitch.h>

ESP8266WebServer server(80);
RCSwitch rcSwitch = RCSwitch();


struct Configuration {

  char mqttServer[30];
  char mqttUser[30];
  char mqttPassword[30];
  int mqttPort = 14985;
  char wifiSSID[30];
  char wifiPassword[30];

} configuration;

WiFiClient espClient;
PubSubClient client(espClient);

uint8_t ledPin = 2;


void setup() {

  Serial.begin(115200);
  setupTimeClient();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  rcSwitch.enableReceive(D2);

  rcSwitch.enableTransmit(D1);

  loadDataDefault();

  configureServer();

  setupWiFi();
}


void loop() {


  if (WiFi.status() == WL_CONNECTED) {

    if (client.connected()) {

      client.loop();
    } else {

      loopConnectMQTT();
    }
  } else {

    loopConnectWifi();
  }

  loopRF();
  server.handleClient();


  float te = 25 * 100;

  char *strTemp = dec2binWzerofill(te, 16);


  char teId[16] = "100000000000011";

  teId[15] = '\0';
  char sendingTe[32];
  sprintf(sendingTe, "%s%s", strTemp, teId);

  rcSwitch.send(sendingTe);

  updateTriggerServo();
}

void loopRF() {

  if (rcSwitch.available()) {

    const char* rawBinany = dec2binWzerofill(rcSwitch.getReceivedValue(), 30);

    char valueBinary[16];
    for (int i = 0 ; i < 15 ; i++) valueBinary[i] = rawBinany[i];
    valueBinary[15] = '\0';
    Serial.println(valueBinary);
    ///
    char idBinary[16];
    for (int i = 0 ; i < 15 ; i++) idBinary[i] = rawBinany[i + 15];
    idBinary[15] = '\0';
    Serial.println(idBinary);
    Serial.println(rawBinany);

    long valueLong = strtol(valueBinary, NULL, 2);
    float valueFloat = valueLong / 100.00;
    char mystr[6];

    dtostrf(valueFloat, 4, 2, mystr);

    //    sprintf(mystr,"%s",);


    long idLong = strtol(idBinary, NULL, 2);
    char idStr[10];
    sprintf(idStr, "%lu", idLong);

    client.publish(idStr, mystr);

    rcSwitch.resetAvailable();

  }
}

char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0) {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}

#include <Servo.h>

Servo myservo;
#define CONTROL_PIN D7
int ser_pos_feeder = 70;
int ser_pos_fishtank = 50;
int pos = 0;    // variable to store the servo position

#include <NTPClient.h>
#include "Timer.h"
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
WatchDog watchDog;

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  char *charArray = (char*)payload;
  String str = (String)charArray;
  Serial.print(str);

  if (strcmp(topic, "switch") == 0) {

    if (str.equals("1")) {
      digitalWrite(ledPin, LOW);
      activateServo();
      client.publish(topic, "0", true);
      delay(100);
      client.publish(topic, "done", false);

    } else if (str.equals("0"))  {

      digitalWrite(ledPin, HIGH);
    } else if (str.equals("done")) {
      //      client.publish(topic, "0", true);
    }
  }

  if (strcmp(topic, "openvalue") == 0) {

    if (str.equals("done")) {
      Serial.println("#done");
    } else {

      ser_pos_fishtank = str.toInt();
      Serial.print("set value open: ");
      Serial.println(ser_pos_fishtank);
      client.publish(topic, "done", false);
    }
  }

  if (strcmp(topic, "timetrigger") == 0) {

    if (str.equals("done")) {
      Serial.println("#done");
    } else {

      watchDog.setTimeString(str);
      Serial.print("trigger string: ");
      Serial.println(str);
      client.publish(topic, "done", false);
    }
  }

  Serial.println("ok");
  Serial.println("-----------------------");
  //  digitalWrite(ledPin, !digitalRead(ledPin));

}

void setupTimeClient() {
  timeClient.begin();
}

void updateTriggerServo() {
  timeClient.update();
  bool isActive = watchDog.isAlarmAtTime(timeClient.getHours(), timeClient.getMinutes());
  if (isActive) {
    Serial.println("Activate servo");
    activateServo();
  }
}

void activateServo() {

  Serial.print("openvalue = : ");
  Serial.println(ser_pos_fishtank);

  myservo.attach(CONTROL_PIN);  // attaches the servo on pin 9 to the servo object
  myservo.write(ser_pos_feeder);
  for (pos = ser_pos_feeder; pos >= ser_pos_fishtank ; pos -= 1) {
    myservo.write(pos);              // tell servo to go to position in variable 'pos'

  }
  delay(1000);

  for (pos = ser_pos_fishtank; pos <= ser_pos_feeder; pos += 1) {
    // in steps of 1 degree
    myservo.write(pos);               // tell servo to go to position in variable 'pos'

  }
  delay(1000);

  myservo.detach();

}

void setupWiFi() {
  delay(500);
  String host = "esp" + String(ESP.getChipId());;
  MDNS.begin(host);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  delay(500);

//  const char* ssid = "khuonvienxanh";
  String ssid = "ESP" + String(ESP.getChipId());

  WiFi.softAP(ssid.c_str());
  IPAddress myIP = WiFi.softAPIP();

  long now = millis();

  while ((millis() - now) < 30000) {

    server.handleClient();

  }
}

void loadDataDefault() {

  DataDefault<Configuration> dataDefault;
  configuration = dataDefault.loadObject();
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

    content += "<h1>SSID:  " ;
    content += configuration.wifiSSID ;
    content += "</h1> <br>";

    content += "<h1>Password:  " ;
    content += configuration.wifiPassword ;
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

  });

  server.on("/wifisetting", []() {

    String ssid = server.arg("ssid");
    String wifiPass = server.arg("wifiPassword");

    strcpy( configuration.wifiSSID, ssid.c_str());
    strcpy( configuration.wifiPassword, wifiPass.c_str());


    DataDefault<Configuration> dataDefault;
    dataDefault.saveObject(configuration);

    String content = "<html>";
    content += "<body>";
    content += "<h2>Your Wifi:  <h1>" ;
    //      content += fileId ;
    content += "</h1>";
    content += "has been saved.";
    content += "</h2>" ;
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);

    connectWifi();

  });

  server.on("/", handleRoot);

  server.on("/mqtt", handleMQTTRoot);

  server.begin();
}

void handleRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/wifi.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

void handleMQTTRoot() {
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/index.htm", "r");
  server.streamFile(index, "text/html");
  index.close();
}

long lastReconnectWifiAttempt = 0;
void loopConnectWifi() {

  long now = millis();
  if (now - lastReconnectWifiAttempt > 15000) {
    lastReconnectWifiAttempt = now;
    connectWifi();
    if (WiFi.status() == WL_CONNECTED) {
      lastReconnectWifiAttempt = 0;
    }
  }
}

void connectWifi() {

  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(configuration.wifiSSID, configuration.wifiPassword);
  Serial.print("SSID: ");
  Serial.println(configuration.wifiSSID);
  Serial.print("Password: ");

  Serial.println(configuration.wifiPassword);

  Serial.print("wifi connecting...");
}

long lastReconnectMQTTAttempt = 0;

void loopConnectMQTT() {

  long now = millis();
  if (now - lastReconnectMQTTAttempt > 60000) {

    lastReconnectMQTTAttempt = now;
    // Attempt to connect
    connectMQTT();

    if (client.connected()) {
      lastReconnectMQTTAttempt = 0;
    }
  }
}

void connectMQTT() {

  delay(500);
  if (client.connected()) {
    return;
  }
  client.setServer(configuration.mqttServer, configuration.mqttPort);
  client.setCallback(callback);

  Serial.println(configuration.mqttServer);
  Serial.println(configuration.mqttPort);
  Serial.println(configuration.mqttUser);
  Serial.println(configuration.mqttPassword);

  Serial.println("Connecting to MQTT...");

  if (client.connect("ESP8266Client", configuration.mqttUser, configuration.mqttPassword )) {

    Serial.println("connected");
    client.subscribe("switch", 0);
    client.subscribe("openvalue", 1);
    client.subscribe("timetrigger", 1);
    client.publish("FirstPing", "Hello ");
  } else {

    Serial.print("failed with state ");
    Serial.print(client.state());
  }
}
