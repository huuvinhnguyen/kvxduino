#if defined(ESP8266)
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library         
#else
#include <WiFi.h>      //ESP32 Core WiFi Library    
#endif


#if defined(ESP8266)

#include <ESP8266WebServer.h>  //Local WebServer used to serve the configuration portal

#else

#include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )
#endif

#include <DNSServer.h> //Local WebServer used to serve the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )
#include <WiFiManager.h>   // WiFi Configuration Magic (  https://github.com/zhouhan0126/DNSServer---esp32  ) >>  https://github.com/zhouhan0126/DNSServer---esp32  (ORIGINAL)    //https://github.com/tzapu/WiFiManager


const int PIN_AP = 2;
void setup() {
  Serial.begin(9600);
  pinMode(PIN_AP, INPUT);
  //declaração do objeto wifiManager
  WiFiManager wifiManager;

//utilizando esse comando, as configurações são apagadas da memória
  //caso tiver salvo alguma rede para conectar automaticamente, ela é apagada.
//  wifiManager.resetSettings();
 
//callback para quando entra em modo de configuração AP
  wifiManager.setAPCallback(configModeCallback); 
//callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
  wifiManager.setSaveConfigCallback(saveConfigCallback); 
 
//cria uma rede de nome ESP_AP com senha 12345678
  wifiManager.autoConnect("ESP_AP", "12345678"); 

}

void loop() {

  WiFiManager wifiManager;
  //se o botão foi pressionado
   if ( digitalRead(PIN_AP) == HIGH ) {
      Serial.println("resetar"); //tenta abrir o portal
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
        Serial.println("Falha ao conectar");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
      Serial.println("Conectou ESP_AP!!!");
   }

}
   //callback que indica que o ESP entrou no modo AP
void configModeCallback (WiFiManager *myWiFiManager) {  
//  Serial.println("Entered config mode");
  Serial.println("Entrou no modo de configuração");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede</p><p>}</p><p>//callback que indica que salvamos uma nova rede para se conectar (modo estação)
}
void saveConfigCallback () {
//  Serial.println("Should save config");
  Serial.println("Configuração salva");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
}

