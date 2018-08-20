
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

 
#define CS 15                   //define chip select line for manual control

ESP8266WebServer server(80);


const char* ssid="The Coffee House"; 
const char* password="thecoffeehouse"; 

String webSite,javaScript,XML;
int LED=5;
int start=0;

void buildWebsite(){
  buildJavascript();
  webSite="<!DOCTYPE HTML>\n";
  webSite+=javaScript;
  webSite+="<HTML>\n";
  webSite+="<style>\n";
  webSite+="#button {\n";
  webSite+="background-color: #E6E6FA;\n";
  webSite+="border: none;\n";
  webSite+="color: white;\n";
  webSite+="padding: 32px;\n";
  webSite+=" text-align: center;\n";
  webSite+=" text-decoration: none;\n";
  webSite+="display: inline-block;\n";
  webSite+="font-size: 168px;\n";
  webSite+="display:block;\n";
  webSite+="margin:0 auto;\n";
  webSite+="margin-top:130px;\n";
  webSite+="cursor: pointer;\n";
  webSite+="width:440px;\n";
  webSite+="height:400px;\n";
  webSite+="}\n";


  
  webSite+="p.thicker{font-weight:900;}\n";
  webSite+="#runtime{font-weight:900; font-size: 147%; color:RED;}\n";
  webSite+="</style>\n";
  webSite+="<BODY bgcolor='#E6E6FA' onload='process()'>\n";
    
  //webSite+="<div>Change the state of LED! </div>\n";
  webSite+="<button onClick='RunButtonWasClicked()' id='button'></button>  ";
  
  

  webSite+="</BODY>\n";
  webSite+="</HTML>\n";
}

void buildJavascript(){
  javaScript="<SCRIPT>\n";
  
  javaScript+="var xmlHttp=createXmlHttpObject();\n";
  javaScript+="function createXmlHttpObject(){\n";
  javaScript+=" if(window.XMLHttpRequest){\n";
  javaScript+="    xmlHttp=new XMLHttpRequest();\n";
  javaScript+=" }else{\n";
  javaScript+="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  javaScript+=" }\n";
  javaScript+=" return xmlHttp;\n";
  javaScript+="}\n";
  
  javaScript+="var click;\n";
  
  javaScript+="function handleServerResponse(){\n";
  javaScript+="   xmlResponse=xmlHttp.responseXML;\n";
  javaScript+="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  javaScript+="   message = xmldoc[0].firstChild.nodeValue;\n";
  javaScript+="if(message == 1){click = 1; message = 'ON'; document.getElementById('button').style.background='#FFA200';}else{click=0; message='OFF'; document.getElementById('button').style.background='#111111';}\n";
  javaScript+="   document.getElementById('button').innerHTML=message;\n";
  javaScript+="}\n";

  javaScript+="function process(){\n";
  javaScript+="   xmlHttp.open('PUT','xml',true);\n";
  javaScript+="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
  javaScript+="   xmlHttp.send(null);\n";
  javaScript+=" setTimeout('process()',200);\n";
  javaScript+="}\n";

  javaScript+="function process2(){\n";
  javaScript+="    xmlHttp.open('SET','set1ESPval?Start='+click,true);\n";
  javaScript+="    xmlHttp.send(null);\n";
  javaScript+=" setTimeout('process2()',400);\n";
  javaScript+="}\n";

  javaScript+="function RunButtonWasClicked(){\n";
  javaScript+="click = (click==1)?0:1;\n";
  javaScript+="    xmlHttp.open('SET','set1ESPval?Start='+click,true);\n";
  javaScript+="    xmlHttp.send(null);\n";
  javaScript+="}\n";       

  javaScript+="</SCRIPT>\n";
}
uint16_t x;
String data;
void buildXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>";
  XML+=data;
  XML+="</response>";
}


void handleWebsite(){
  buildWebsite();
  server.send(200,"text/html",webSite);
}

void handleXML(){
  buildXML();
  server.send(200,"text/xml",XML);
}

void handle1ESPval(){
  start = server.arg("Start").toFloat();
 // Serial.println(start);
 // buildXML();
 // server.send(200,"text/xml",XML);

}
int start2=0;

int inc=0;

void setup() {

 
  Serial.begin(9600);  
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED)delay(500);
  WiFi.mode(WIFI_STA);
  Serial.println("\n\nBOOTING ESP8266 ...");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/",handleWebsite);
  server.on("/xml",handleXML);
  server.on("/set1ESPval",handle1ESPval);
  
  server.begin();  
  pinMode(LED,OUTPUT);

}

void loop() {

    digitalWrite(LED,!start);
  //  Serial.println(start);
    data =(String)start;
    server.handleClient();
} 
