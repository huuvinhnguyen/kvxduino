// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include "ViewInteractor.h"
// #include "DataDefault.h"
// #include <ESP8266mDNS.h>
// #include "Relay.h"
// #include "secrets.h"
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>
#include <SlaveDevice.h>

#include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ESP_DoubleResetDetector.h>

#include <ESPmDNS.h>


const int PIR_SENSOR_OUTPUT_PIN = 3;
#define LED_BUILTIN 8

const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds

int countDevice = 0;

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

WiFiClient net;
PubSubClient client(net);


void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupBLE();
  Serial.println("setup");


}

void loop() {
  Serial.println("loop");

//    if (WiFi.status() == WL_CONNECTED) {
//      if (client.connected()) {
//        client.loop();
//      } else {
//        loopConnectMQTT();
//      }
//    } else {
//      loopConnectWiFi();
//    }


  loopConnectBLE();

  delay(1000);

}

void loopConnectWiFi() {

}

void loopConnectMQTT() {

}

// Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

void loopConnectBLE() {

 

  //  unsigned long currentMillis = millis();

  SlaveDevice &dev = slaveDevices[countDevice];
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
    BLEDevice::getScan()->start(0); // Start scanning again indefinitely

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


void setupWiFi() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.println("BLEAdvertisedDeviceCallbacks--- onResult advertisedDevice");

      SlaveDevice* dev = getDeviceByName(advertisedDevice.getName().c_str());
      if (dev != nullptr) {
        advertisedDevice.getScan()->stop(); // Scan can be stopped, we found what we are looking for
        dev->pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
        Serial.println("Device found. Connecting!");
        dev->doConnect = true;
      }
    }
};


void setupBLE() {

  BLEDevice::init("ESP32_Central_Hub");
  for (auto &device : slaveDevices) {
    Serial.println("device.createClient");
    device.createClient();
  }

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5);

  countDevice = 0;

}
