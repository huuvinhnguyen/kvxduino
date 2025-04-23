const int PIR_SENSOR_OUTPUT_PIN = 3;
using PirCallback = void(*)(void);


class Pir {
  public:
    static volatile bool pirTriggered;  // Biến cờ để tránh gọi callback trong ISR
    static PirCallback callbackFunc;
    static volatile unsigned long lastTriggerTime;

    void setupPir() {
      pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_OUTPUT_PIN), pir, RISING);  /* Interrupt on rising edge on pin 3 */
    }

    void loopPir() {
      if (pirTriggered) {
        pirTriggered = false;  // Reset cờ ngay lập tức để tránh lặp lại liên tục
        Serial.println("Object Detected");

        if (callbackFunc) {  // Kiểm tra callback trước khi gọi
          callbackFunc();
        }
      }
    }
  
    static void IRAM_ATTR pir() {
      unsigned long now = millis();
      if (now - lastTriggerTime > 5000) { // Chỉ kích hoạt nếu cách lần trước ít nhất 500ms
        lastTriggerTime = now;
        pirTriggered = true;
      }
    }

    void registerCallback(PirCallback callback) {
      callbackFunc = callback;
    }
};

volatile bool Pir::pirTriggered = false;
volatile unsigned long Pir::lastTriggerTime = 0;
PirCallback Pir::callbackFunc = nullptr;
