#include <SlaveDevice.h>
#include <set>
#include <iostream>
#include <string>
#include <stack>

// Định nghĩa một kiểu dữ liệu cho deviceInfo
typedef std::unordered_map<std::string, std::string> DeviceInfo;

class DeviceInfoStack {
  private:
    std::stack<DeviceInfo> deviceStack;

    bool isDeviceInStack(const std::string& address) {
      std::stack<DeviceInfo> tempStack = deviceStack;
      while (!tempStack.empty()) {
        if (tempStack.top().at("address") == address) {
          return true;
        }
        tempStack.pop();
      }
      return false;
    }

  public:
    void pushDevice(const DeviceInfo& device) {
      if (!isDeviceInStack(device.at("address"))) {
        deviceStack.push(device);
      }
    }

    bool isEmpty() const {
      return deviceStack.empty();
    }

    DeviceInfo popDevice() {
      if (!deviceStack.empty()) {
        DeviceInfo device = deviceStack.top();
        deviceStack.pop();
        return device;
      }
      return DeviceInfo(); // Trả về một DeviceInfo rỗng nếu stack trống
    }

    DeviceInfo top() const {
      if (!deviceStack.empty()) {
        return deviceStack.top();
      }
      return DeviceInfo(); // Trả về một DeviceInfo rỗng nếu stack trống
    }

    void printDevices() {
      std::stack<DeviceInfo> tempStack = deviceStack;
      while (!tempStack.empty()) {
        DeviceInfo device = tempStack.top();
        std::cout << "Device Name: " << device["name"] << std::endl;
        std::cout << "Device Address: " << device["address"] << std::endl;
        tempStack.pop();
      }
    }
};

DeviceInfoStack* stack = new DeviceInfoStack();

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

      DeviceInfo deviceInfo;
      deviceInfo["name"] = advertisedDevice.getName().c_str();
      if (!deviceInfo["name"].empty()) {
        Serial.println("onResult AAAAAAAA");
        deviceInfo["address"] = advertisedDevice.getAddress().toString().c_str();
        deviceInfo["doconnect"] = "doconnect";
        stack->pushDevice(deviceInfo);

        Serial.println("Device found. Connecting!");

      }
    }
};

typedef void (*BLENotifyCallback)(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                  uint8_t* pData, size_t length, bool isNotify);

class BLEConnector {
  private:
    BLENotifyCallback notifyCallbackFunc;
    BLEScan* pBLEScan;
    bool scanning = false;
    unsigned long scanStartTime = 0;

    SlaveDevice slaveDevices[3] = {
      SlaveDevice("91bad492-b950-4226-aa2b-4ede9fa42f59", "2c3f30b8-1fa1-4d63-8c19-f50a3a7a2ec8", "ESP32_BME280"),
      SlaveDevice("11bad492-b950-4226-aa2b-4ede9fa42f55", "333f30b8-1fa1-4d63-8c19-f50a3a7a2ef9", "ESP32_PIR"),
      SlaveDevice("11bad492-b950-4226-aa2b-4ede9fa42f51", "333f30b8-1fa1-4d63-8c19-f50a3a7a2ef6", "ESP32_PIR2"),
    };

    SlaveDevice* getDeviceByName(const std::string& name) {
      for (auto& device : slaveDevices) {
        if (device.deviceName == name) {
          return &device;
        }
      }
      return nullptr; // Return nullptr if not found
    }

  public:

    void loopConnectBLE() {

      if (stack->isEmpty()) {
        Serial.println("Stack is empty, starting BLE scan...");
        long now = millis();

        if (scanning) { // Kiểm tra xem việc quét có đang diễn ra không
          Serial.println("noscanning...");
          // Kiểm tra nếu đã qua 30 giây từ khi bắt đầu quét
          if (now - scanStartTime > 30000) {
            scanning = false;
            scanStartTime = 0;
            Serial.println("Scan timed out, resetting scan status...");
          } else {

            Serial.println("waiting...");

          }
        } else {

          Serial.println("is scanning...");

          BLEDevice::getScan()->clearResults();
          pBLEScan->start(1, false); // Thêm false để không gọi lại liên tục
          scanning = true; // Đánh dấu rằng việc quét đang diễn ra
          scanStartTime = now;

        }
        return;
      }

      // Khi có thiết bị trong stack, dừng việc quét nếu cần thiết
      if (scanning) {
        pBLEScan->stop();
        scanning = false;
      }

      Serial.println("Processing device from stack...");

      DeviceInfo deviceInfo = stack->top();
      std::string deviceName = deviceInfo["name"];
      std::string address = deviceInfo["address"];
      Serial.println(deviceInfo["name"].c_str());


      SlaveDevice* dev2 = getDeviceByName(deviceName);
      if (dev2 == nullptr) {
        Serial.println("Device not found in slaveDevices.");
        stack->popDevice(); // Loại bỏ thiết bị không hợp lệ khỏi stack
        return;
      }
      Serial.println("CCCCCCccccc");

      if (dev2->pServerAddress != nullptr) {
        delete dev2->pServerAddress;
      }

      dev2->pServerAddress = new BLEAddress(address.c_str());
      dev2->registerNotifyCallback(notifyCallbackFunc);

      Serial.println("Device Name:  ");
      Serial.print(dev2->deviceName.c_str());
      if (deviceInfo["doconnect"] == "doconnect") {
        dev2->doConnect = true;
      }

      if(dev2->bleClient->isConnected()) { return;}

      if (dev2->doConnect) {
        Serial.println("Do connecting ble");
        if (dev2->connect()) {
 
          Serial.println("We are now connected to the BLE Server.");
          //          dev.remoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
        } else {
          Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");

        }
        
        Serial.println("pop device");
        stack->popDevice();
        dev2->doConnect = false;

      }
    }

    void setupBLE() {

      BLEDevice::init("ESP32_Central_Hub");
      setupBLEScan();

    }

    void setupBLEScan() {

      for (auto &device : slaveDevices) {
        Serial.println("device.createClient");
        device.createClient();
      }

      pBLEScan = BLEDevice::getScan();
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setActiveScan(true);

    }

    void registerNotifyCallback(BLENotifyCallback callback) {
      notifyCallbackFunc = callback;
    }
};
