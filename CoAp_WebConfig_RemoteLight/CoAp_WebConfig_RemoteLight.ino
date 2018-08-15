#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <coap.h>
#include <WiFiManager.h>

//
//const char* ssid = "Ving";
//const char* password = "123456789";
//WiFiServer server(80);


// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

// CoAP server endpoint url callback
void callback_light(CoapPacket &packet, IPAddress ip, int port);

// UDP and CoAP class
WiFiUDP udp;
Coap coap(udp);

// LED STATE
bool LEDSTATE;

const int buttonPin = 3;
int buttonState = 0;

// CoAP server endpoint URL
void callback_light(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");
  
  // send response
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  String message(p);

  if (message.equals("0"))
    LEDSTATE = false;
  else if(message.equals("1"))
    LEDSTATE = true;
      
  if (LEDSTATE) {
    digitalWrite(2, HIGH) ; 
    coap.sendResponse(ip, port, packet.messageid, "1");
  } else { 
    digitalWrite(2, LOW) ; 
    coap.sendResponse(ip, port, packet.messageid, "0");
  }
}

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

void setup() {
  
  Serial.begin(115200);



  setupWiFi();
  
////  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//      delay(500);
//      Serial.print(".");
//  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // LED State
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  LEDSTATE = true;

   pinMode(buttonPin, INPUT);

//  
  // add server url endpoints.
  // can add multiple endpoint urls.
  // exp) coap.server(callback_switch, "switch");
  //      coap.server(callback_env, "env/temp");
  //      coap.server(callback_env, "env/humidity");
  Serial.println("Setup Callback Light");
  coap.server(callback_light, "light");

  // client response callback.
  // this endpoint is single callback.
  Serial.println("Setup Response Callback");
  coap.response(callback_response);

  // start coap server/client
  coap.start();
}

void loop() {

    buttonState = digitalRead(buttonPin);


  if (buttonState == LOW) {

    int msgid = coap.put(IPAddress(192, 168, 1, 32), 5683, "light", "0");
 
  } else {
    int msgid = coap.put(IPAddress(192, 168, 1, 32), 5683, "light", "1");
  }

    int msgid = coap.get(IPAddress(192, 168, 1, 96), 5683, "time");

  // send GET or PUT coap request to CoAP server.
  // To test, use libcoap, microcoap server...etc
  // int msgid = coap.put(IPAddress(10, 0, 0, 1), 5683, "light", "1");



  delay(1000);
  coap.loop();
}

void setupWiFi() {
//    WiFi.disconnect(true);
  delay(10000);
  
   WiFiManager wifiManager;

    //reset saved settings
//    WiFi.disconnect(true);
//delay(2000);
//ESP.reset();
//    ESP.eraseConfig();
//    delay(3000);
    wifiManager.setConfigPortalTimeout(60);
    wifiManager.startConfigPortal();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
//    wifiManager.autoConnect("VingESP");
    //or use this for auto generated name ESP + ChipID
//    wifiManager.autoConnect(); 
 }
/*
if you change LED, req/res test with coap-client(libcoap), run following.
coap-client -m get coap://(arduino ip addr)/light
coap-client -e "1" -m put coap://(arduino ip addr)/light
coap-client -e "0" -m put coap://(arduino ip addr)/light
*/

/*
https://github.com/hirotakaster/CoAP-simple-library/blob/master/examples/coaptest/coaptest.ino
*/
