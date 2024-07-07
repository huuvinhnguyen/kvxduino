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

const int PIR_SENSOR_OUTPUT_PIN = 3;

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

// Create a sensor object
// Init BME280
void initBME(){
  
}
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

void setupPir() {
  pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), pir, RISING);  /* Interrupt on rising edge on pin 13 */
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
  Serial.println("Waiting a client connection to notify...");

}

void pir(){
  Serial.println("Object Detected");
  int32_t value = 1; // Set the value to 1

  pressureCharacteristic.setValue((uint8_t *)&value, 4); // Set the value as 4 bytes
  pressureCharacteristic.notify(); // Notify connected client
}
void setup() {
  // Start serial communication 
  Serial.begin(115200);

    // Initialize the LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Ensure the LED is off at the start

  // Start BME sensor
  initBME();
  setupBLE();
  setupPir();
}

uint8_t value = 0;

void loop() {
  if (deviceConnected) {
  
    pressureCharacteristic.setValue((uint8_t *)&value, 4);
    pressureCharacteristic.notify();
    value++;
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  } else {

    digitalWrite(LED_BUILTIN, HIGH);
  }

}
