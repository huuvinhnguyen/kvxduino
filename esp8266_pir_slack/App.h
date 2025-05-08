#if defined(ESP32)
  #include <Preferences.h>
  Preferences prefs;
#elif defined(ESP8266)
  #include <EEPROM.h>
  #define EEPROM_SIZE 512
  const int OFFLINE_FLAG_ADDR = 0; // Địa chỉ lưu bool
#endif

class App {
  public:
    static void setup(); // Gọi 1 lần trong setup() để khởi tạo
    static bool isOfflineMode();
    static void setOfflineActive(bool offline);
};

void App::setup() {
#if defined(ESP32)
  prefs.begin("app", false); // namespace "app", read-write
#elif defined(ESP8266)
  EEPROM.begin(EEPROM_SIZE);
#endif
}

bool App::isOfflineMode() {
#if defined(ESP32)
  return prefs.getBool("offline_mode", false); // false nếu chưa có
#elif defined(ESP8266)
  bool value;
  EEPROM.get(OFFLINE_FLAG_ADDR, value);
  return value;
#endif
}

void App::setOfflineActive(bool offline) {
#if defined(ESP32)
  prefs.putBool("offline_mode", offline);
#elif defined(ESP8266)
  EEPROM.put(OFFLINE_FLAG_ADDR, offline);
  EEPROM.commit();
#endif
}
