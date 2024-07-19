#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>

//using BLENotifyCallback = void(*)(int32_t);

using BLENotifyCallback = void(*)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);



class SlaveDevice {
  private:
    //    static BLENotifyCallback notifyCallbackFunc;
    static BLENotifyCallback notifyCallbackFunc;

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

      if (notifyCallbackFunc) {
        notifyCallbackFunc(pBLERemoteCharacteristic, pData, length, isNotify);
      }

    }

    bool connect() {

      if (bleClient == nullptr) {
        bleClient = BLEDevice::createClient();
      }

      Serial.println(" - Before connect");
      bleClient->disconnect();
      delay(100);
      if (!bleClient->connect(*pServerAddress)) {
          return false;
        }
      Serial.println(" - Connected to server ");


      remoteService = bleClient->getService(serviceUUID.c_str());
      Serial.println(" - Connected to server   1 ");

      if (remoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        bleClient->disconnect();
        return false;
      }

            Serial.println(" - Connected to server   2 ");


      remoteCharacteristic = remoteService->getCharacteristic(characteristicUUID.c_str());
      if (remoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(characteristicUUID.c_str());
        bleClient->disconnect();
        return false;
      }

      Serial.println(" - Found our characteristics");

      // Assign callback functions for the Characteristics
      if(notifyCallback) {
              remoteCharacteristic->registerForNotify(notifyCallback);

        } else {
            return false;
          }
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
      SlaveDevice::notifyCallbackFunc  = callback;
    }

};

BLENotifyCallback SlaveDevice::notifyCallbackFunc = nullptr;
//BLENotifyCallback2 SlaveDevice::notifyCallbackFunc2 = nullptr;
