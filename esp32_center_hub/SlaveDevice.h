#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
//#include <BLEAdvertisedDevice.h>

//using BLENotifyCallback = void(*)(BLERemoteCharacteristic*, uint8_t*, size_t, bool);
using BLENotifyCallback = void(*)(String jsonString);


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
    static String receivedData;// To store the received fragments


    SlaveDevice(std::string serviceUUID, std::string charUUID, std::string charName)
      : serviceUUID(serviceUUID), characteristicUUID(charUUID), bleClient(nullptr), lastAttemptTime(0), deviceName(charName) {}

    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                               uint8_t* pData, size_t length, bool isNotify) {

      String fragment = String((char*)pData).substring(0, length);
      receivedData += fragment;
      Serial.println("fragment: ");
      Serial.println(fragment);
      if (receivedData.endsWith("}")) {  // Assuming JSON data always ends with '}'

        StaticJsonDocument<200> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, receivedData);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.c_str());
        } else {
          uint32_t chipId = jsonDoc["id"];
          const char* sensor = jsonDoc["sen"];
          float temperature = jsonDoc["tem"];
          float humidity = jsonDoc["hum"];

        }

        if (notifyCallbackFunc && isNotify) {
          //        notifyCallbackFunc(pBLERemoteCharacteristic, pData, length, isNotify);
          notifyCallbackFunc(receivedData);
        }

        receivedData = "";  // Clear the buffer after processing
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

String SlaveDevice::receivedData = "";
BLENotifyCallback SlaveDevice::notifyCallbackFunc = nullptr;
