#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>

using BLENotifyCallback = void(*)(int32_t);

class SlaveDevice {
  private:
    static BLENotifyCallback notifyCallbackFunc;

    SlaveDevice() {
      // Khởi tạo các thành viên
    }

  public:

    BLEAdvertisedDevice* advertisedDevice;
    BLEAddress *pServerAddress;
    std::string serviceUUID;
    std::string characteristicUUID;
    BLEClient* bleClient;
    BLERemoteService* remoteService;
    BLERemoteCharacteristic* remoteCharacteristic;
    bool doConnect = false;

    float temperature;
    float humidity;
    float pressure;
    std::string deviceName;

    unsigned long lastAttemptTime;


    SlaveDevice(std::string serviceUUID, std::string charUUID, std::string charName)
      : serviceUUID(serviceUUID), characteristicUUID(charUUID), bleClient(nullptr),
        remoteService(nullptr), remoteCharacteristic(nullptr), temperature(0.0),
        humidity(0.0), pressure(0.0), lastAttemptTime(0), deviceName(charName) {}

    // When the BLE Server sends a new pressure reading with the notify property
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                               uint8_t* pData, size_t length, bool isNotify) {
      // Store pressure value
      int32_t notifyValue;

      memcpy(&notifyValue, pData, sizeof(notifyValue));
      Serial.println("notify value: ");
      Serial.print(notifyValue);

      if (notifyCallbackFunc) {
        Serial.println("static callback");
        notifyCallbackFunc(notifyValue);
      }

    }

    bool connect() {
      bleClient = BLEDevice::createClient();

      Serial.println(" - Before connect");
      bleClient->connect(*pServerAddress);
      Serial.println(" - Connected to server ");

      remoteService = bleClient->getService(serviceUUID.c_str());
      if (remoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        bleClient->disconnect();
        return false;
      }

      remoteCharacteristic = remoteService->getCharacteristic(characteristicUUID.c_str());
      if (remoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(characteristicUUID.c_str());
        bleClient->disconnect();
        return false;
      }

      Serial.println(" - Found our characteristics");

      // Assign callback functions for the Characteristics
      remoteCharacteristic->registerForNotify(notifyCallback);
      return true;
    }

    void createClient() {
      Serial.println("Setup created client");
      bleClient = BLEDevice::createClient();

    }

    void retrieveData() {
      if (remoteCharacteristic->canRead()) {
        // std::string value = remoteCharacteristic->readValue();
        // Parse the value according to your data format
        // Example: assuming value contains temperature, humidity, and pressure
        // Parse and assign these values to temperature, humidity, pressure
      }
    }

    void registerNotifyCallback(BLENotifyCallback callback) {
      Serial.println("ble callback level 1");
      SlaveDevice::notifyCallbackFunc  = callback;
    }

};

BLENotifyCallback SlaveDevice::notifyCallbackFunc = nullptr;
