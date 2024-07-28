#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
//#include <BLEAdvertisedDevice.h>

using BLENotifyCallback = void(*)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);

// Client callbacks
class MyClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) override {
      Serial.println("Connected to BLE server.");
    }

    void onDisconnect(BLEClient* pClient) override {
      Serial.println("Disconnected from BLE server.");
    }
};

class SlaveDevice {
  private:
    static BLENotifyCallback notifyCallbackFunc;
    BLERemoteCharacteristic* remoteCharacteristic;

  public:
    BLEAdvertisedDevice* advertisedDevice;
    BLEAddress* pServerAddress = nullptr;
    std::string serviceUUID;
    std::string characteristicUUID;
    BLEClient* bleClient;

    bool doConnect = false;
    std::string deviceName;

    unsigned long lastAttemptTime = 0;

    SlaveDevice(std::string serviceUUID, std::string charUUID, std::string charName)
      : serviceUUID(serviceUUID), characteristicUUID(charUUID), bleClient(nullptr), lastAttemptTime(0), deviceName(charName) {}

    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                               uint8_t* pData, size_t length, bool isNotify) {
      if (notifyCallbackFunc && isNotify) {
        notifyCallbackFunc(pBLERemoteCharacteristic, pData, length, isNotify);
      }
    }

    bool connect() {
      if (bleClient == nullptr) {
        bleClient = BLEDevice::createClient();

      }

      Serial.println(" - Before connect");
      if (pServerAddress == nullptr) {
        Serial.println("pServerAddress is null");
        return false;
      }
      
      bleClient->disconnect();
      delay(1000);
      if (!bleClient->connect(*pServerAddress)) {
        return false;
      }
      Serial.println(" - Connected to server ");

      BLERemoteService* remoteService = bleClient->getService(serviceUUID.c_str());
      Serial.println(" - Connected to server 1 ");

      if (remoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        bleClient->disconnect();
        return false;
      }

      Serial.println(" - Connected to server 2 ");

      remoteCharacteristic = remoteService->getCharacteristic(characteristicUUID.c_str());
      if (remoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(characteristicUUID.c_str());
        bleClient->disconnect();
        return false;
      }

      if (!remoteCharacteristic->canNotify()) {
        Serial.println("Cannot notify");
        bleClient->disconnect();
        return false;
      }

      // Assign callback functions for the Characteristics
      if (notifyCallback) {
        Serial.println(" - registerForNotify");
        remoteCharacteristic->registerForNotify(notifyCallback, true);

      } else {
        Serial.println(" - registerForNotifyFail");
        return false;
      }
      Serial.println(" - Found our characteristics");

      return true;
    }

    void createClient() {
      Serial.println("Setup created client");
      bleClient = BLEDevice::createClient();
      bleClient->setClientCallbacks(new MyClientCallbacks());
    }

    void registerNotifyCallback(BLENotifyCallback callback) {
      SlaveDevice::notifyCallbackFunc = callback;
    }
};

BLENotifyCallback SlaveDevice::notifyCallbackFunc = nullptr;
