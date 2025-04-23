#include "Relay.h"


void Relay::setup(uint8_t relayPin) {

  pin = relayPin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, RELAY_OFF);

}


void Relay::loop(callbackFunc func) {
  cb1 = func;

  unsigned long now = millis();  // Đảm bảo sử dụng unsigned long để tránh overflow

  if (isSetOnLastingActive) {
    // Nếu đây là lần đầu tiên chạy (startAttempedTime == 0), khởi tạo thời gian bắt đầu
    if (startAttempedTime == 0) {
      startAttempedTime = now;
      setOn(true);  // Bật relay ngay lập tức
      Serial.println("Relay bật lần đầu.");
      cb1("relay_on", value);  // Gửi callback thông báo relay đã tắt

    }

    // Kiểm tra nếu đã vượt quá thời gian longlast
    if (now - startAttempedTime >= longlast) {
      setOn(false);  // Tắt relay
      startAttempedTime = 0;  // Reset thời gian
      isSetOnLastingActive = false;
      Serial.println("Relay tắt sau thời gian đã định.");
      cb1("relay_off", value);  // Gửi callback thông báo relay đã tắt

    }
  }
}


void Relay::setOn(bool isOn) {
  uint8_t relayValue = (isOn) ? RELAY_ON : RELAY_OFF;
  digitalWrite(pin, relayValue);

  value = isOn;

}


void Relay::switchOn() {
  isSetOnLastingActive = true;
  startAttempedTime = 0;           // Đặt lại thời gian bắt đầu (cho phép bật tức thì)


}

void Relay::setLonglast(int seconds) {
  longlast = seconds * 1000;
}
