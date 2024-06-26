/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include "BLEDevice.h"
#include <Wire.h>

// BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32_BME280"

// Define the pin for the built-in LED
#define LED_BUILTIN 2  // Adjust this pin according to your setup

// Define the pin for the LED
#define LED_PIN 13  // Adjust this pin according to your setup

/* UUID's of the service, characteristic that we want to read */
// BLE Service
static BLEUUID bmeServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");

// Characteristic
static BLEUUID characteristicUUID("2c3f30b8-1fa1-4d63-8c19-f50a3a7a2ec8");

// Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

// BLEClient that we will use to connect
static BLEClient* pClient = nullptr;

// Characteristic that we want to read
static BLERemoteCharacteristic* characteristic;

// Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

// Variables to store pressure
char* pressureChar;

int32_t pressureValue;

// Flags to check whether new pressure readings are available
boolean newPressure = false;

// Variables for LED timing
boolean ledOn = false;
unsigned long ledOnTime = 0;
const unsigned long ledDuration = 5000; // LED on duration in milliseconds

// Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  // Connect to the remote BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bmeServiceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  characteristic = pRemoteService->getCharacteristic(characteristicUUID);
//   characteristic = pRemoteService->getCharacteristics()->first;
  if (characteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristics");

  // Assign callback functions for the Characteristics
  characteristic->registerForNotify(notifyCallback);

  return true;
}

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.println("advertisedDevice.getName() = ");
    Serial.println(advertisedDevice.getName());

    if (advertisedDevice.getName() == bleServerName) { // Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); // Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
      doConnect = true; // Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};

// When the BLE Server sends a new pressure reading with the notify property
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                           uint8_t* pData, size_t length, bool isNotify) {
  // Store pressure value
  pressureChar = (char*)pData;
  newPressure = true;
  memcpy(&pressureValue, pData, sizeof(pressureValue));
  Serial.println("notify value: ");
  Serial.print(pressureValue);

  if (pressureValue == 1) {
    turnOnLed();
  }
}

// Function to turn on the LED for 5 seconds
void turnOnLed() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_PIN, HIGH);  // Turn on the LED
  ledOnTime = millis();  // Record the time when the LED was turned on
  ledOn = true;  // Set the LED on flag
}

// Function that prints the latest sensor readings
void printReadings() {
  Serial.print("receive value: ");
  Serial.println(pressureChar);
  if (strcmp(pressureChar, "OK") == 0) {
//    turnOnLed();
  }
}

void setupBLE() {
   // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  // Init BLE device
  BLEDevice::init("");

  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Ensure the LED is off at the start

  // Initialize the LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Ensure the LED is off at the start

  setupBLE();

}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect. Now we connect to it. Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      // Activate the Notify property of the Characteristic
      characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }

  // If the connection is lost, try to reconnect
  if (!pClient->isConnected()) {
    connected = false;
    Serial.println("Disconnected from server. Scanning for new connection...");
//    BLEDevice::getScan()->start(0); // Start scanning again indefinitely
    setupBLE();
  }

  // If new pressure readings are available, print them
  if (newPressure) {
    newPressure = false;
    printReadings();
  }

  // Check if the LED needs to be turned off
  if (ledOn && (millis() - ledOnTime >= ledDuration)) {
    digitalWrite(LED_PIN, LOW);  // Turn off the LED
    digitalWrite(LED_BUILTIN, LOW);
    ledOn = false;  // Reset the LED on flag
  }


  delay(1000); // Delay a second between loops
}
