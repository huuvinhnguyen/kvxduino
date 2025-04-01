#include <EEPROM.h>

template <class T>
class DataDefault {
  public:
    // Lấy chuỗi từ EEPROM
    String stringFromEEPROM() {
      String string = "";
      for (int i = 0; i < EEPROM.length(); ++i) {
        char c = char(EEPROM.read(i));
        if (c == '\0') break;  // Dừng khi gặp ký tự kết thúc chuỗi
        string += c;
      }
      return string;
    }

    // Lưu chuỗi vào EEPROM
    void saveEEPROM(String content) {
      int maxLength = 96;
      int length = content.length();

      if (length > maxLength) {
        Serial.println("Error: Content too long for EEPROM");
        return;
      }

      // Xóa dữ liệu cũ
      for (int i = 0; i < maxLength; ++i) {
        EEPROM.write(i, 0);
      }

      // Ghi dữ liệu mới
      for (int i = 0; i < length; ++i) {
        EEPROM.write(i, content[i]);
        Serial.print("Wrote: ");
        Serial.println(content[i]);
      }

      EEPROM.commit();
    }

    // Lưu đối tượng vào EEPROM
    void saveObject(T t) {
      EEPROM.put(0, t);
      EEPROM.commit();
    }

    // Tải đối tượng từ EEPROM
    T loadObject() {
      T t;
      EEPROM.get(0, t);
      return t;
    }

    void setup() {
      Serial.println("Initializing EEPROM...");
      EEPROM.begin(5000);  // Khởi tạo EEPROM một lần duy nhất
    }

    String readEEPROMString(int startAddr) {
      String data = "";
      for (int i = startAddr; i < startAddr + eepromStringLength(startAddr); i++) {
        char c = EEPROM.read(i);
        if (c == '\0') break;  // Dừng khi gặp ký tự kết thúc chuỗi
        data += c;
      }
      return data;
    }

    void writeEEPROMString(int startAddr, const String& data, int maxLength) {
      int length = min((int)data.length(), maxLength - 1);  // Chuyển đổi kiểu cho đúng

      for (int i = 0; i < length; i++) {
        EEPROM.write(startAddr + i, data[i]);
      }
      EEPROM.write(startAddr + length, '\0');  // Null-terminate string
      EEPROM.commit();
    }


    void clearEEPROMString(int startAddr) {
      int maxLength = eepromStringLength(startAddr);
      for (int i = 0; i < maxLength; i++) {
        EEPROM.write(startAddr + i, '\0');  // Ghi ký tự null để xóa
      }
      EEPROM.commit();
    }

    int eepromStringLength(int startAddr) {
      int length = 0;
      while (EEPROM.read(startAddr + length) != '\0' && length < 512) {
        length++;
      }
      return length + 1;
    }
};
