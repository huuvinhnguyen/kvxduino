#include <WiFiManager.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define WIFI_SSID_ADDR 0
#define WIFI_PASS_ADDR 32

class WiFiHandler {
  public:

    void setupWiFi() {

      EEPROM.begin(EEPROM_SIZE);

      WiFiManager wifiManager;

      //exit after config instead of connecting
      wifiManager.setBreakAfterConfig(true);

      //reset settings - for testing
      //wifiManager.resetSettings();


      //tries to connect to last known settings
      //if it does not connect it starts an access point with the specified name
      //here  "AutoConnectAP" with password "password"
      //and goes into a blocking loop awaiting configuration
      wifiManager.setConfigPortalTimeout(180);

      if (!wifiManager.autoConnect("AutoConnectAP")) {
        Serial.println("failed to connect, we should reset as see if it connects");
        delay(3000);
        ESP.restart();
        delay(5000);
      }

      saveCredentials(wifiManager.getWiFiSSID(), wifiManager.getWiFiPass());
      Serial.println("Connected through Wi-Fi Manager.");

      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
      Serial.println("local ip");
      Serial.println(WiFi.localIP());
    }

    long lastReconnectWifiAttempt = 0;
    void loopConnectWiFi() {

      if (WiFi.status() == WL_CONNECTED) {
      } else {
        long now = millis();
        if (now - lastReconnectWifiAttempt > 15000) {
          lastReconnectWifiAttempt = now;

          WiFi.mode(WIFI_STA);
          String ssid = readEEPROM(WIFI_SSID_ADDR);
          String pass = readEEPROM(WIFI_PASS_ADDR);
          WiFi.begin(ssid.c_str(), pass.c_str());

          if (WiFi.status() == WL_CONNECTED) {
            lastReconnectWifiAttempt = 0;
          }
        }
      }

    }

  private:
    void saveCredentials(const String& ssid, const String& pass) {
      writeEEPROM(WIFI_SSID_ADDR, ssid);
      writeEEPROM(WIFI_PASS_ADDR, pass);
      EEPROM.commit();
      Serial.println("Credentials saved to EEPROM.");
    }

    String readEEPROM(int startAddr) {
      String data = "";
      for (int i = startAddr; i < startAddr + 32; i++) {
        char c = EEPROM.read(i);
        if (c == '\0') break;
        data += c;
      }
      return data;
    }

    void writeEEPROM(int startAddr, const String& data) {
      for (int i = 0; i < data.length(); i++) {
        EEPROM.write(startAddr + i, data[i]);
      }
      EEPROM.write(startAddr + data.length(), '\0');  // Null-terminate string
    }
};
