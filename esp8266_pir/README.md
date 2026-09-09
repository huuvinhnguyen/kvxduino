# ESP8266 PIR

Firmware cho ESP8266 đọc cảm biến PIR và gọi API trigger khi phát hiện chuyển động, tích hợp MQTT và WiFiManager.

## Yêu cầu

- macOS, Linux hoặc Windows
- `arduino-cli`
- ESP8266 core cho Arduino
- ESP8266 đã kết nối bằng cáp USB có truyền dữ liệu
- Các thư viện Arduino được project sử dụng:
  - ESP8266WiFi
  - ESP8266WebServer
  - ESP8266HTTPClient
  - ESP8266mDNS
  - PubSubClient
  - ArduinoJson
  - WiFiManager
  - [kvxshared](https://github.com/huuvinhnguyen/kvxshared)

## Cài đặt Arduino CLI

Cài `arduino-cli` theo hướng dẫn chính thức, sau đó kiểm tra:

```bash
arduino-cli version
```

Cài ESP8266 core nếu máy chưa có:

```bash
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
```

Kiểm tra core:

```bash
arduino-cli core list
```

## Chuẩn bị thư viện

Cài các thư viện còn thiếu bằng `arduino-cli lib install`:

```bash
arduino-cli lib install "PubSubClient"
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "WiFiManager"
```

Cài thủ công thư viện `kvxshared` từ GitHub vào thư mục thư viện Arduino:

```bash
cd "$(arduino-cli config get directories.user)/libraries"
git clone https://github.com/huuvinhnguyen/kvxshared.git
```

Nếu thư viện đã được clone trước đó, cập nhật bằng:

```bash
cd "$(arduino-cli config get directories.user)/libraries/kvxshared"
git pull
```

Sau khi cài đặt, khởi động lại Arduino IDE nếu đang mở.

## Build firmware

Tạo thư mục build:

```bash
mkdir -p build
```

Build firmware:

```bash
arduino-cli compile \
  --fqbn esp8266:esp8266:nodemcuv2 \
  --output-dir ./build \
  .
```

Board mặc định: `esp8266:esp8266:nodemcuv2` (NodeMCU v2)

Có thể chọn board khác:

```bash
arduino-cli compile \
  --fqbn esp8266:esp8266:d1_mini \
  --output-dir ./build \
  .
```

Firmware được tạo trong thư mục `build/`:

- `esp8266_pir.ino.bin`: file firmware để nạp
- `esp8266_pir.ino.elf`: file debug
- `esp8266_pir.ino.map`: thông tin vùng nhớ

## Tìm cổng USB

Kết nối ESP8266 rồi chạy:

```bash
arduino-cli board list
```

Trên macOS, cổng thường có dạng:

```text
/dev/cu.usbserial-XXXXXXXX
```

Không chọn các cổng Bluetooth như `/dev/cu.BLTH` hoặc `/dev/cu.Bluetooth-Incoming-Port`.

## Nạp firmware

Build và nạp trong một lệnh:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --output-dir ./build . && \
arduino-cli upload \
  --fqbn esp8266:esp8266:nodemcuv2 \
  --port /dev/cu.usbserial-A5069RR4 \
  --input-dir ./build \
  .
```

Thay `/dev/cu.usbserial-A5069RR4` bằng cổng thực tế của ESP8266.

Nếu cần xem chi tiết quá trình nạp:

```bash
arduino-cli upload --verbose \
  --fqbn esp8266:esp8266:nodemcuv2 \
  --port /dev/cu.usbserial-A5069RR4 \
  --input-dir ./build \
  .
```

Khi thành công, log sẽ có các dòng tương tự:

```text
Writing at ... (100 %)
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

## Kiểm tra sau khi nạp

Mở Serial Monitor ở tốc độ `115200`:

```bash
arduino-cli monitor \
  --port /dev/cu.usbserial-A5069RR4 \
  --config baudrate=115200
```

Khi PIR phát hiện chuyển động, firmware sẽ in:

```text
PIR detected, sending trigger
```

Sau đó firmware gọi API:

```text
POST https://khuonvien.vn/api/devices/trigger
```

Payload gửi lên có dạng:

```json
{
  "chip_id": "esp8266_<chip_id>"
}
```

## Kết nối phần cứng PIR

Firmware sử dụng chân:

```text
PIR OUT -> D7 (GPIO13)
PIR VCC -> 3.3V hoặc 5V (tùy PIR module)
PIR GND -> GND
```

PIR được xử lý theo cạnh lên: mỗi lần tín hiệu chuyển từ `LOW` sang `HIGH`, firmware gọi API một lần. Khi PIR vẫn giữ `HIGH`, firmware không gọi lặp liên tục.

Delay giữa các lần đọc: `500ms`

## Tính năng chính

### 1. Phát hiện chuyển động

Firmware đọc trạng thái PIR từ chân D7 và so sánh với trạng thái trước đó. Khi phát hiện cạnh lên (LOW → HIGH), gọi `AppApi::sendTrigger()`.

### 2. Kết nối Wi-Fi

Sử dụng `WiFiHandler` để quản lý kết nối Wi-Fi. Firmware hỗ trợ WiFiManager cho cấu hình Wi-Fi qua web portal khi chưa có thông tin mạng.

### 3. MQTT

Firmware kết nối tới MQTT broker và lắng nghe các topic:

- `<device_id>/refresh`: đồng bộ dữ liệu từ server
- `<device_id>/update_version`: cập nhật firmware OTA
- `<device_id>/reset_wifi`: reset cấu hình Wi-Fi

Khi kết nối MQTT thành công, firmware gửi thông tin reset reason lên topic `<device_id>`.

### 4. OTA Update

Firmware hỗ trợ cập nhật OTA qua URL được cấu hình từ server. URL update được lưu trong `App::setUpdateUrl()` và sử dụng khi nhận lệnh `update_version`.

### 5. Đồng bộ server

Hàm `syncServerData()` thực hiện:
- Lấy thông tin device từ API
- Cập nhật URL firmware
- Gửi build version và app version lên server

## API endpoints

Firmware sử dụng các API:

### Send Trigger
```text
POST /api/devices/trigger
Body: {"chip_id": "esp8266_<chip_id>"}
```

### Get Device Info
```text
GET /api/devices/<device_id>
```

### Update Last Seen
```text
POST /api/devices/last_seen
Body: {"build_version": <int>, "app_version": "<string>"}
```

## Xử lý lỗi thường gặp

### Không thấy cổng USB

- Kiểm tra cáp USB có hỗ trợ truyền dữ liệu
- Rút và cắm lại ESP8266
- Chạy lại `arduino-cli board list`
- Cài driver USB-UART phù hợp với chip trên board (CH340 hoặc CP210x)

### Lỗi `D7 was not declared`

Đảm bảo đang build bằng board profile có hỗ trợ alias chân ESP8266:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --output-dir ./build .
```

### Upload bị treo ở `Connecting...`

- Nhấn giữ nút `FLASH` hoặc `BOOT` trên board
- Bấm nút `RST` một lần
- Thả `FLASH` khi bắt đầu upload
- Đảm bảo không có Serial Monitor hoặc ứng dụng khác đang giữ cổng USB

### Lỗi thiếu thư viện

Đọc tên thư viện trong log build và cài thư viện tương ứng:

```bash
arduino-cli lib install "TEN_THU_VIEN"
```

### API không được gọi

Kiểm tra:

- ESP8266 đã kết nối Wi-Fi (xem log Serial Monitor)
- Serial Monitor có dòng `PIR detected, sending trigger`
- Server API đang hoạt động
- PIR OUT thực sự thay đổi từ `LOW` sang `HIGH`
- Kiểm tra `Free Heap` trong Serial Monitor để đảm bảo không bị tràn bộ nhớ

### MQTT không kết nối

- Kiểm tra `App::mqttHost` và `App::mqttPort` đúng
- Kiểm tra firewall không chặn cổng MQTT
- Xem log Serial Monitor để biết trạng thái kết nối MQTT

## Cấu trúc code

- `esp8266_pir.ino`: file chính chứa setup và loop
- `WiFiHandler`: quản lý kết nối Wi-Fi
- `MQTTHandler`: quản lý kết nối MQTT và publish/subscribe
- `MQTTMessageHandler`: xử lý message MQTT nhận được
- `AppApi`: các hàm gọi API
- `App`: cấu hình app và device ID
- `Esp8266Server`: web server cho cấu hình (nếu có)

## Device ID

Device ID được tạo từ chip ID của ESP8266:

```cpp
String deviceId = "esp8266_" + String(ESP.getChipId(), HEX);
```

Device ID này được sử dụng cho tất cả API calls và MQTT topics.