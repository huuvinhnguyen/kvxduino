#include <WiFiManager.h>

class WiFiHandler {
  public:

    void setupWiFi() {
      //WiFiManager
      //Local intialization. Once its business is done, there is no need to keep it around
      WiFiManager wifiManager;

      //exit after config instead of connecting
      wifiManager.setBreakAfterConfig(true);

      //reset settings - for testing
      //wifiManager.resetSettings();


      //tries to connect to last known settings
      //if it does not connect it starts an access point with the specified name
      //here  "AutoConnectAP" with password "password"
      //and goes into a blocking loop awaiting configuration
      if (!wifiManager.autoConnect("AutoConnectAP")) {
        Serial.println("failed to connect, we should reset as see if it connects");
        delay(3000);
        ESP.restart();
        delay(5000);
      }

      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
      Serial.println("local ip");
      Serial.println(WiFi.localIP());
    }

    long lastReconnectWifiAttempt = 0;
    void loopConnectWiFi() {
      long now = millis();
      if (now - lastReconnectWifiAttempt > 15000) {
        lastReconnectWifiAttempt = now;
        WiFi.disconnect();
        delay(100);
        WiFi.reconnect();
        if (WiFi.status() == WL_CONNECTED) {
          lastReconnectWifiAttempt = 0;
        }
      }

    }
};
