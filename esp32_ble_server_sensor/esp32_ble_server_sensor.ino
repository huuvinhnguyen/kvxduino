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
void setup() {
  // Start serial communication 
  Serial.begin(115200);
  // Start BME sensor
  initBME();
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
void loop() {
  if (deviceConnected) {
  
    float p = random(0, 10000) / 100.0;
    //Notify temperature reading

    uint16_t pressure = (uint16_t)p;
    //Set humidity Characteristic value and notify connected client
    pressureCharacteristic.setValue(pressure);
    pressureCharacteristic.notify();   
    Serial.print("Pressure: ");
    Serial.print(p);
    Serial.println(" hPa");
    delay(1000);
  }

}
