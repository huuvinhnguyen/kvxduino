#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Define the pin for the built-in LED
#define LED_BUILTIN 8  // Adjust this pin according to your setup

BLEServer *pServer;

//BLE server name
#define bleServerName "ESP32_DHT"
// Default UUID for Environmental Sensing Service
// https://www.bluetooth.com/specifications/assigned-numbers/
static BLEUUID bmeServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");

static BLEUUID characteristicUUID("2c3f30b8-1fa1-4d63-8c19-f50a3a7a2ec8");

// Pressure Characteristic and Descriptor
BLECharacteristic characteristic(characteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor descriptor(BLEUUID((uint16_t)0x2902));

bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device Connected");
      pServer->startAdvertising();

    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device Disconnected");
      // Start advertising again
      pServer->startAdvertising();
      Serial.println("Advertising started");
    }
};

void setupDHT() {
  dht.begin();
}

void setupBLE() {
  // Create the BLE Device
  BLEDevice::init(bleServerName);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *bmeService = pServer->createService(bmeServiceUUID);

  bmeService->addCharacteristic(&characteristic);
  characteristic.addDescriptor(&descriptor);
  // Start the service
  bmeService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  2          /* Time ESP32 will go to sleep (in seconds) */
uint32_t getChipId() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  setupBLE();
  setupDHT();

  // Configure the ESP32 to wake up using a timer after TIME_TO_SLEEP seconds
  //  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  if (deviceConnected) {
    loopDHT();
  } else {
    // If not connected, keep advertising and wait for a connection
    delay(1000); // Wait for a connection
  }
}



void loopDHT() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  uint32_t chipId = getChipId();

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = chipId;
  jsonDoc["sen"] = "dht11";
  jsonDoc["tem"] = t;
  jsonDoc["hum"] = h;
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  if (deviceConnected) {
    // Split the JSON string into smaller chunks if necessary
    size_t chunkSize = 20;  // BLE notification size limit
    for (size_t i = 0; i < jsonString.length(); i += chunkSize) {
      String chunk = jsonString.substring(i, min(i + chunkSize, jsonString.length()));
      characteristic.setValue(chunk.c_str());
      characteristic.notify();
      delay(10);  // Short delay to avoid overloading the BLE stack
    }
    Serial.println("Notification sent: " + jsonString);

  } else {
    Serial.println("Device not connected, data not sent.");
  }

  delay(1000);
  //  esp_light_sleep_start();

}
