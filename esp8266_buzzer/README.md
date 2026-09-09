# ESP8266 Buzzer Control

Hệ thống điều khiển buzzer qua MQTT sử dụng ESP8266, hỗ trợ bật/tắt thủ công và tự động tắt sau một khoảng thời gian.

## Tính năng

- **Điều khiển qua MQTT**: Nhận lệnh từ MQTT broker để bật/tắt buzzer
- **Tự động tắt**: Buzzer tự động tắt sau khoảng thời gian đã định
- **Điều khiển thủ công**: Có thể bật/tắt buzzer ngay lập tức
- **Tích hợp relay timer**: Hỗ trợ điều khiển relay kèm theo
- **OTA update**: Cập nhật firmware từ xa qua MQTT
- **WiFi auto-reconnect**: Tự động kết nối lại WiFi khi mất kết nối

## Phần cứng

- **Board**: ESP8266
- **Buzzer pin**: D7 (GPIO13)
- **Điện áp**: 3.3V/5V tùy loại buzzer

## Cấu trúc thư viện

### BuzzerTimer Class

Class quản lý hoạt động của buzzer với khả năng tự động tắt.

#### Phương thức chính

**`void setup(uint8_t buzzerPin)`**
- Khởi tạo buzzer với pin được chỉ định
- Thiết lập pin ở chế độ OUTPUT
- Đặt trạng thái ban đầu là OFF

**`void loop()`**
- Gọi trong vòng lặp chính để kiểm tra timeout
- Tự động tắt buzzer khi hết thời gian

**`void activateFor(int longlast)`**
- Bật buzzer trong khoảng thời gian `longlast` (milliseconds)
- Buzzer sẽ tự động tắt sau khi hết thời gian
- Ví dụ: `buzzerTimer.activateFor(5000)` → buzzer bật trong 5 giây

**`void setOn(bool isOn)`**
- Bật/tắt buzzer ngay lập tức
- `true`: bật buzzer
- `false`: tắt buzzer
- Hủy timer tự động tắt khi gọi

## Cách sử dụng

### 1. Khởi tạo trong setup()

```cpp
BuzzerTimer buzzerTimer;

void setup() {
  buzzerTimer.setup(D7);  // Sử dụng pin D7
}
```

### 2. Gọi loop() trong vòng lặp chính

```cpp
void loop() {
  buzzerTimer.loop();  // Xử lý tự động tắt
  delay(500);
}
```

### 3. Điều khiển qua MQTT

Hệ thống nhận lệnh qua topic: `{deviceId}/switchon`

#### Payload JSON

**Bật buzzer trong thời gian xác định:**
```json
{
  "longlast": 5000
}
```
→ Buzzer bật trong 5 giây rồi tự tắt

**Bật/tắt buzzer thủ công:**
```json
{
  "switch_value": true
}
```
→ Bật buzzer ngay lập tức

```json
{
  "switch_value": false
}
```
→ Tắt buzzer ngay lập tức

**Kết hợp với reminder:**
```json
{
  "longlast": 3000,
  "reminder": "Nhắc nhở quan trọng"
}
```
→ Buzzer bật 3 giây và lưu message nhắc nhở

## MQTT Topics

| Topic | Mô tả | Payload |
|-------|-------|---------|
| `{deviceId}/switchon` | Điều khiển buzzer | `{"longlast": 5000}` hoặc `{"switch_value": true}` |
| `{deviceId}/switchon/relay` | Trạng thái relay (publish) | `{"value": true, "index": 0}` |
| `{deviceId}/refresh` | Đồng bộ dữ liệu từ server | - |
| `{deviceId}/update_version` | Cập nhật firmware OTA | - |
| `{deviceId}/reset_wifi` | Reset cấu hình WiFi | - |

## Cấu hình

### WiFi & MQTT

Cấu hình trong file `app.h`:
- MQTT host và port
- Device ID
- Topics đăng ký

### Pin Configuration

```cpp
buzzerTimer.setup(D7);  // GPIO13 trên ESP8266
```

Có thể thay đổi pin bằng cách truyền pin khác vào `setup()`.

## Dependencies

- ESP8266WiFi
- ArduinoJson
- PubSubClient (trong MQTTHandler)

## Luồng hoạt động

1. ESP8266 kết nối WiFi
2. Kết nối tới MQTT broker
3. Subscribe các topics điều khiển
4. Nhận message từ MQTT:
   - Nếu có `longlast`: bật buzzer và đặt timer tự động tắt
   - Nếu có `switch_value`: bật/tắt buzzer ngay lập tức
5. `buzzerTimer.loop()` kiểm tra và tự động tắt khi hết thời gian

## Xử lý sự kiện

### Khi MQTT connected
- Đồng bộ dữ liệu từ server
- Gửi reset reason và device ID
- Cập nhật thông tin relay timer

### Khi relay thay đổi
- Publish trạng thái mới lên MQTT
- Topic: `{deviceId}/switchon/relay`

## Lưu ý

- Buzzer sử dụng logic HIGH để bật (active HIGH)
- Timer sử dụng `millis()` nên sẽ overflow sau ~50 ngày
- Delay 500ms trong loop chính → độ chính xác timeout ±500ms
- Khi gọi `setOn()` sẽ hủy timer tự động tắt đang chạy

## Ví dụ sử dụng

### Bật buzzer 2 giây
```cpp
buzzerTimer.activateFor(2000);
```

### Tắt buzzer ngay lập tức
```cpp
buzzerTimer.setOn(false);
```

### Bật buzzer và giữ mãi
```cpp
buzzerTimer.setOn(true);
```
