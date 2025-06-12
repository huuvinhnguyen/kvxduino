#if defined(ESP32)
#include <Preferences.h>
#elif defined(ESP8266)
#include <EEPROM.h>
#define EEPROM_SIZE 512
const int OFFLINE_FLAG_ADDR = 0;
const int UPDATE_URL_ADDR = 10;
const int UPDATE_URL_MAX_LEN = 200;
#endif

class App {
  public:
    static String mqttHost;
    static int mqttPort;
    static int buildVersion;
    static String appVersion;
    static void setup(); // Gọi 1 lần trong setup()
    static String getDeviceId();
    static bool isOfflineMode();
    static void setOfflineActive(bool offline);
    static String getUpdateUrl();
    static void setUpdateUrl(const String& url);
    static const char* const topicActions[];
    static String getResetReasonString();

  private:
    static uint32_t getChipId();
    static String updateUrl;

#if defined(ESP32)
    static Preferences& getPrefs();

#endif


};

void App::setup() {
#if defined(ESP32)
  getPrefs().begin("app", false); // namespace "app"
#elif defined(ESP8266)
  EEPROM.begin(EEPROM_SIZE);
#endif
}

bool App::isOfflineMode() {
#if defined(ESP32)
  return getPrefs().getBool("offline_mode", false);
#elif defined(ESP8266)
  bool value = false;
  EEPROM.get(OFFLINE_FLAG_ADDR, value);
  return value;
#endif
}

void App::setOfflineActive(bool offline) {
#if defined(ESP32)
  getPrefs().putBool("offline_mode", offline);
#elif defined(ESP8266)
  EEPROM.put(OFFLINE_FLAG_ADDR, offline);
  EEPROM.commit();
#endif
}

#if defined(ESP32)
Preferences& App::getPrefs() {
  static Preferences prefs;
  return prefs;
}
#endif

String App::getUpdateUrl() {
  return App::updateUrl;
}

void App::setUpdateUrl(const String& url) {
  App::updateUrl = url;
}


String App::getDeviceId() {
#if defined(ESP8266)
  return "esp8266_" + String(getChipId());
#elif defined(ESP32)
  return "esp32_" + String(getChipId());
#else
  return "unknown_device";
#endif
}

uint32_t App::getChipId() {
#if defined(ESP32)
  uint64_t mac = ESP.getEfuseMac();  // chỉ gọi 1 lần
  uint32_t chipId = 0;
  for (int i = 0; i < 24; i += 8) {
    chipId |= ((mac >> (40 - i)) & 0xff) << i;
  }
  return chipId;
#elif defined(ESP8266)
  return ESP.getChipId();
#else
#error "Unsupported platform. This code supports only ESP32 and ESP8266."
#endif
}

String App::getResetReasonString() {
  esp_reset_reason_t reason = esp_reset_reason();

  switch (reason) {
    case ESP_RST_POWERON:    return "POWER_ON";
    case ESP_RST_EXT:        return "EXTERNAL_RESET";
    case ESP_RST_SW:         return "SOFTWARE_RESET";
    case ESP_RST_PANIC:      return "PANIC_RESET";
    case ESP_RST_INT_WDT:    return "INTERRUPT_WATCHDOG";
    case ESP_RST_TASK_WDT:   return "TASK_WATCHDOG";
    case ESP_RST_BROWNOUT:   return "BROWNOUT";
    case ESP_RST_SDIO:       return "SDIO_RESET";
    case ESP_RST_DEEPSLEEP:  return "DEEP_SLEEP";
    default:                 return "UNKNOWN";
  }
}


String App::updateUrl = "";
int App::buildVersion = 0;
String App::appVersion = "1.0.0";
String App::mqttHost = "103.9.77.155";
int App::mqttPort = 1883;
const char* const App::topicActions[] = {
  "switch",
  "switchon",
  "timetrigger",
  "longlast",
  "ping",
  "refresh",
  "restart",
  "set_offline_mode",
  "update_version",
  "reset_wifi"
};
