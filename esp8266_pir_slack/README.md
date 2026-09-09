# ESP8266 PIR Slack

Firmware cho ESP8266 đọc cảm biến PIR và gọi API trigger khi phát hiện chuyển động.

## Yêu cầu

- macOS, Linux hoặc Windows
- `arduino-cli`
- ESP8266 core cho Arduino
- ESP8266 đã kết nối bằng cáp USB có truyền dữ liệu
- Các thư viện Arduino được project sử dụng:
  - ESP8266WiFi
  - ESP8266WebServer
  - ESP8266HTTPClient
  - PubSubClient
  - ArduinoJson
  - WiFiManager
  - WebSockets
  - arduino-ds1302-master

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

Cài các thư viện còn thiếu bằng `arduino-cli lib install`, hoặc cài qua Arduino IDE. Ví dụ:

```bash
arduino-cli lib install "PubSubClient"
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "WiFiManager"
arduino-cli lib install "WebSockets"
```

Thư viện `arduino-ds1302-master` cần được cài thủ công nếu chưa có trong thư mục thư viện Arduino.

## Build firmware

Mở terminal tại thư mục project:

```bash
cd /path/to/esp8266_pir_slack
```

Cấp quyền thực thi cho script một lần:

```bash
chmod +x build.sh
```

Build bằng board mặc định NodeMCU v2:

```bash
./build.sh
```

Firmware được tạo trong thư mục `build/`, thường gồm:

- `esp8266_pir_slack.ino.bin`: file firmware để nạp
- `esp8266_pir_slack.ino.elf`: file debug
- `esp8266_pir_slack.ino.map`: thông tin vùng nhớ

Board mặc định trong script:

```text
esp8266:esp8266:nodemcuv2
```

Có thể chọn board khác bằng biến `FQBN`:

```bash
FQBN=esp8266:esp8266:d1_mini ./build.sh
```

## Tìm cổng USB

Kết nối ESP8266 rồi chạy:

```bash
arduino-cli board list
```

Trên macOS, cổng thường có dạng:

```text
/dev/cu.usbserial-XXXXXXXX
```

Không chọn các cổng Bluetooth như:

```text
/dev/cu.BLTH
/dev/cu.Bluetooth-Incoming-Port
```

## Nạp firmware

Build và nạp trong một lệnh:

```bash
./build.sh && arduino-cli upload \
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

Firmware hiện sử dụng chân:

```text
PIR OUT -> D7
PIR VCC -> 3.3V
PIR GND -> GND
```

PIR được xử lý theo cạnh lên: mỗi lần tín hiệu chuyển từ `LOW` sang `HIGH`, firmware gọi API một lần. Khi PIR vẫn giữ `HIGH`, firmware không gọi lặp liên tục.

## Xử lý lỗi thường gặp

### Không thấy cổng USB

- Kiểm tra cáp USB có hỗ trợ truyền dữ liệu.
- Rút và cắm lại ESP8266.
- Chạy lại `arduino-cli board list`.
- Cài driver USB-UART phù hợp với chip trên board, thường là CH340 hoặc CP210x.

### Lỗi `D5`, `D6`, `D7 was not declared`

Đảm bảo đang build bằng board profile có hỗ trợ alias chân ESP8266, ví dụ:

```bash
FQBN=esp8266:esp8266:nodemcuv2 ./build.sh
```

### Upload bị treo ở `Connecting...`

- Nhấn giữ nút `FLASH` hoặc `BOOT` trên board.
- Bấm nút `RST` một lần.
- Thả `FLASH` khi bắt đầu upload.
- Đảm bảo không có Serial Monitor hoặc ứng dụng khác đang giữ cổng USB.

### Lỗi thiếu thư viện

Đọc tên thư viện trong log build và cài thư viện tương ứng:

```bash
arduino-cli lib install "TEN_THU_VIEN"
```

### API không được gọi

Kiểm tra:

- ESP8266 đã kết nối Wi-Fi.
- Serial Monitor có dòng `PIR detected, sending trigger`.
- Server API đang hoạt động.
- PIR OUT thực sự thay đổi từ `LOW` sang `HIGH`.
