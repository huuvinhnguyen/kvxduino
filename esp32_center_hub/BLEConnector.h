#include <SlaveDevice.h>


SlaveDevice slaveDevices[] = {
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

const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //      Serial.println("BLEAdvertisedDeviceCallbacks--- onResult advertisedDevice");

      SlaveDevice* dev = getDeviceByName(advertisedDevice.getName().c_str());
      if (dev != nullptr) {
        //advertisedDevice.getScan()->stop(); // Scan can be stopped, we found what we are looking for
        dev->pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
        Serial.println("Device found. Connecting!");
        dev->bleClient->disconnect();
        dev->doConnect = true;
      }
    }
};

typedef void (*BLENotifyCallback)(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                               uint8_t* pData, size_t length, bool isNotify);
//using BLENotifyCallback = void(*)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);

class BLEConnector {
  private:
    BLENotifyCallback notifyCallbackFunc;

  public:

    int countDevice = 0;

    void loopConnectBLE() {

      //  unsigned long currentMillis = millis();

      SlaveDevice &dev = slaveDevices[countDevice];
      dev.registerNotifyCallback(notifyCallbackFunc);

      Serial.println("Device Name:  ");
      Serial.print(dev.deviceName.c_str());
      if (dev.doConnect == true) {

        if (dev.connect()) {
          Serial.println("We are now connected to the BLE Server.");
          dev.remoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
        } else {
          Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");

        }

        dev.doConnect = false;

      }

      if (!dev.bleClient->isConnected()) {
        Serial.println("Disconnected from server. Scanning for new connection...");
        BLEDevice::getScan()->start(1); // Start scanning again indefinitely

      }

      size_t slaveDevicesCount = sizeof(slaveDevices) / sizeof(slaveDevices[0]);
      Serial.println("DevicesCount: ");
      Serial.print(countDevice);

      if (countDevice < slaveDevicesCount - 1) {
        countDevice++;

      } else {
        countDevice = 0;
      }
    }

    void setupBLE() {

      BLEDevice::init("ESP32_Central_Hub");
      for (auto &device : slaveDevices) {
        Serial.println("device.createClient");
        device.createClient();
      }

      BLEScan* pBLEScan = BLEDevice::getScan();
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setActiveScan(true);
      pBLEScan->setInterval(9999);
      pBLEScan->setWindow(99);
      pBLEScan->start(5);

      countDevice = 0;

    }

    void registerNotifyCallback(BLENotifyCallback callback) {
      notifyCallbackFunc = callback;
    }
};
