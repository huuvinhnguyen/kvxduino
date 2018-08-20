
    #include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//SSID and Password to your ESP Access Point
const char* ssid = "Ving";
const char* password = "123456789";

//const char* ssid= "Newton" ;
//const char* password= "X1t3q567ZO.#";  

int LED=2;                              // PINnumber where your LED is
int websockMillis=50;                    // SocketVariables are sent to client every 50 milliseconds
int sliderVal=60;           
byte brightness;
int c;
int inPin = D9 ;
char data[10];
int i;
ESP8266WebServer server(80); //Server on port 80
WebSocketsServer webSocket=WebSocketsServer(88);
String javaScript,JSONtxt;
unsigned long websockCount=1,wait000=0UL,wait001=0UL;
int LEDmillis=9*(100-sliderVal)+100;
boolean LEDonoff=true;

const char webSite[] PROGMEM = // #xxxxxxxx# are later to be changed in c function WebsiteContent()
R"=====(
    <!DOCTYPE HTML><HTML>
    <META name='viewport' content='width=device-width, initial-scale=1'>
    <html>
      <style>
        .box{
          position:relative;
          float:left;
          margin-right:10px;
          margin-bottom:10px;
          font-size:216%;
          font-weight: bold;
          width:65px;
          height:65px;
          border-radius: 50%;
          padding:5px;
          color:white;
          text-align:center;
          background-color:black;
          }

    </style>
  <body>
  </body>

<SCRIPT>



for (var i = 0;i < 200; i++) 
{
var divTag = document.createElement("div");
divTag.id = "kutu"+(i+1);
divTag.className = 'box';
//divTag1.appendChild(divTag);
document.body.appendChild(divTag);
document.getElementById("kutu"+(i+1)).innerHTML = i+1;
}
  var data,datb; 
  InitWebSocket();
  function InitWebSocket(){
  websock=new WebSocket('ws://'+window.location.hostname+':88/');
  websock.onmessage=function(evt){
  JSONobj=JSON.parse(evt.data);  
  data = (JSONobj.box1); 
  for(var i = 1; i <33; i++){
  datb = data>>(i-1) & 0x01; 
  if(datb == 1){document.getElementById("kutu" + i).style.background='#ADFF2F'}else{document.getElementById("kutu" + i).style='background-color:black;';} 
  }
 }
}
  </SCRIPT>
  </html>     
)=====";

void buildWebsite()
{
}
void buildJavascript()
{
}
String millis2time()
{
  String Time="";
  unsigned long ss;
  byte mm,hh;
  ss=millis()/1000;
  hh=ss/3600;
  mm=(ss-hh*3600)/60;
  ss=(ss-hh*3600)-mm*60;
  if(hh<10)Time+="0";
  Time+=(String)hh+":";
  if(mm<10)Time+="0";
  Time+=(String)mm+":";
  if(ss<10)Time+="0";
  Time+=(String)ss;
  return Time;
}

void handleWebsite()
{
  buildWebsite();
  server.send(200,"text/html",webSite);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t wslength)
{
  String payloadString=(const char *)payload;
  Serial.println(payloadString);
  //Serial.println("payload: '"+payloadString+"', channel: "+(String)num);
  if(type==WStype_TEXT){
  byte separator=payloadString.indexOf('=');
  String var=payloadString.substring(0,separator);
  String val=payloadString.substring(separator+1);
  if(var=="LEDonoff"){
  LEDonoff=false;
  if(val=="ON")LEDonoff=true;
  digitalWrite(LED,HIGH);
  }else if(var=="sliderVal"){
  sliderVal=val.toInt();
  LEDmillis=9*(100-sliderVal)+100;
  }
  }
}

//                  SETUP
//===============================================================
void setup(void)
{
  Serial.begin(115200);
  Serial.println("");
  //WiFi.mode(WIFI_AP); //Only Access point
  WiFi.mode(WIFI_STA);
  //Start HOTspot removing password will disable security
  //WiFi.softAP(ssid, password);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED)delay(500);
  //IPAddress myIP = WiFi.softAPIP(); //Get IP address
  // Serial.print("HotSpt IP:");
  //Serial.println(myIP);
               
  WiFi.mode(WIFI_STA);
  Serial.println("\n\nBOOTING ESP8266 ...");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Station IP address = ");
  Serial.println(WiFi.localIP());
 
  server.on("/",handleWebsite);
  server.begin();
//  webSocket.begin();
//  webSocket.onEvent(webSocketEvent);

  websockCount = 1;
}

char led1=0;
char veri;
char counter =0;

unsigned int senData = 0;
void loop(void)
{  
    webSocket.loop();
    server.handleClient();
    String LEDswitch="OFF";



    LEDonoff = veri & 0x01;
    if(millis()>wait001)
    {
    websockCount++;

  // websockCount = websockCount << 1;
  //  Serial.println(websockCount);
    senData = websockCount;
    String LEDswitch="LED = OFF";
    if(LEDonoff==true)LEDswitch="LED = ON";
    JSONtxt="{\"box1\":\""+(String)(senData & 0xFFFFFFFF)+"\"}";
    webSocket.broadcastTXT(JSONtxt);
    wait001=millis()+websockMillis;
  }
  } 
