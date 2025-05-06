#include <Preferences.h>

class App {
  public:
    static void setup(); // Gọi 1 lần trong setup() để khởi tạo
    static bool isOfflineMode();
    static void setOfflineActive(bool offline);

  private:
    static Preferences prefs; // Biến dùng chung, lưu trữ trên flash
};

// Khởi tạo biến static bên ngoài class
Preferences App::prefs;

void App::setup() {
  prefs.begin("app", false); // namespace "app", read-write
}

bool App::isOfflineMode() {
  return prefs.getBool("offline_mode", false); // false mặc định nếu chưa set
}

void App::setOfflineActive(bool offline) {
  prefs.putBool("offline_mode", offline);
}
