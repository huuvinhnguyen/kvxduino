/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-ble-server-environmental-sensing-service/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

const int PIR_SENSOR_OUTPUT_PIN = 3;
#define DHTPIN 3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Define the pin for the built-in LED
#define LED_BUILTIN 8  // Adjust this pin according to your setup

BLEServer *pServer;

//BLE server name
#define bleServerName "ESP32_BME280"
// Default UUID for Environmental Sensing Service
// https://www.bluetooth.com/specifications/assigned-numbers/
static BLEUUID bmeServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");

static BLEUUID pressureCharacteristicUUID("2c3f30b8-1fa1-4d63-8c19-f50a3a7a2ec8");

// Pressure Characteristic and Descriptor
BLECharacteristic pressureCharacteristic(pressureCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pressureDescriptor(BLEUUID((uint16_t)0x2902));

bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device Connected");
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

  bmeService->addCharacteristic(&pressureCharacteristic);
  pressureCharacteristic.addDescriptor(&pressureDescriptor);
  // Start the service
  bmeService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5          /* Time ESP32 will go to sleep (in seconds) */

void setup() {
  // Start serial communication
  Serial.begin(115200);

  setupBLE();
  setupDHT();

  // Configure the ESP32 to wake up using a timer after TIME_TO_SLEEP seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
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
  // Wait a few seconds between measurements.

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.println(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);

  char dhtDataString[32];
  snprintf(dhtDataString, sizeof(dhtDataString), "{\"temperature\":%.2f,\"humidity\":%.2f}", t, h);
  Serial.println(dhtDataString);
  pressureCharacteristic.setValue(dhtDataString);
  pressureCharacteristic.notify();
  delay(1000);
  esp_deep_sleep_start();

}
