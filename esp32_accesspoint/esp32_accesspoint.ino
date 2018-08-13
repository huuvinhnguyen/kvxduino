#include <WiFi.h>
#include <WiFiAP.h>


const char* ssid     = "Ving"; // the ssid/name of the wifi, the esp will be connected to
const char* password = "123456789"; // the password of that wifi

const char* assid = "espAccessPoint";
const char* asecret = "hello";

void setup(){
  Serial.begin(115200);
  
  WiFi.mode(WIFI_AP_STA);
 

  //access point part
  Serial.println("Creating Accesspoint");
  WiFi.softAP(assid,asecret,7,0,5);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  //station part
  Serial.print("connecting to...");
  Serial.println(ssid);

  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   
  
}

void loop(){
  
 
 }
