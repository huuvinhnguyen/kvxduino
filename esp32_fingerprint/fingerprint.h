#include <Adafruit_Fingerprint.h>
#include "app.h"


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(20, 21);
#else
#define mySerial Serial1
#endif

uint8_t id;
#define RXD2 20 // Pin RX2 del ESP32 (conectar al TX del sensor)
#define TXD2 21 // Pin TX2 del ESP32 (conectar al RX del sensor)


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
using FingerprintCallback = void(*)(char* message);


class Fingerprint {
  private:
    static FingerprintCallback callbackFunc;
    int employeeId = -1;
  public:
    bool enrollmentMode = false; // Tracks if enrollment mode is active
    void registerCallback(FingerprintCallback callback) {
      callbackFunc = callback;
    }
    void setup() {
      mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

      delay(100);
      Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

      finger.begin(57600);

      if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
      } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) {
          delay(1);
        }
      }

      Serial.println(F("Reading sensor parameters"));
      finger.getParameters();
      Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
      Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
      Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
      Serial.print(F("Security level: ")); Serial.println(finger.security_level);
      Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
      Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
      Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
    }

    void loop() {
      Serial.println("Ready to enroll a fingerprint!");

      if (enrollmentMode) {
        int retryCount = 0;
        int maxRetries = 5;  // Maximum retries
        bool enrolledSuccessfully = false;

        // Try enrolling a fingerprint until successful or max retries
        while (retryCount < maxRetries) {
          if (getFingerprintEnroll() == FINGERPRINT_OK) {
            enrolledSuccessfully = true;  // Enrollment succeeded
            break;  // Exit loop after successful enrollment
          } else {
            retryCount++;
            Serial.print("Retry attempt: ");
            Serial.println(retryCount);
          }
        }

        if (enrolledSuccessfully) {
          Serial.println("Fingerprint enrolled successfully!");
        } else {
          Serial.println("Enrollment failed after maximum retries.");
        }

        // Automatically turn off enrollment mode after enrolling or failure
        enrollmentMode = false;
      } else {
        // Normal fingerprint scanning
        getFingerprintID();
      }


    }

    void enroll(int employeeId) {
      this->employeeId = employeeId;
      enrollmentMode = true;
    }

    void cancelEnrollment() {
      Serial.println("cancelEnrollment");
      enrollmentMode = false;
    }

    uint8_t deleteFingerprint(uint8_t id, std::function<void()> callback) {
      uint8_t p = -1;

      p = finger.deleteModel(id);

      if (p == FINGERPRINT_OK) {
        Serial.println("Deleted!");
        callback();
        Serial.println(id);
      } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
      } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not delete in that location");
      } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
      } else {
        Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
      }

      return p;
    }

    void deleteAllFingers() {
      Serial.print("finger.emptyDatabase()");
      finger.emptyDatabase();
    }

  private:
    // Function to find the first available ID
    uint8_t findAvailableID() {
      for (uint8_t id = 1; id <= 127; id++) {
        if (!checkIfIDExists(id)) {
          return id; // Return the first available ID
        }
      }
      return 0; // Return 0 if all IDs are occupied
    }

    // Function to check if an ID is already in use
    bool checkIfIDExists(uint8_t id) {
      uint8_t p = finger.loadModel(id);
      return p == FINGERPRINT_OK; // Returns true if ID exists
    }

    // Function to enroll fingerprint into the AS608 sensor without using an existing ID
    uint8_t getFingerprintEnroll() {
      uint8_t id = findAvailableID();

      if (id == 0) {
        Serial.println("No available slots for enrollment.");
        return 0;
      }

      Serial.print("Enrolling fingerprint ID #");
      Serial.println(id);

      int p = -1;
      Serial.println("Place your finger on the sensor...");
      callbackFunc("fingerprint_touch_1");

      // Step 1: Capture first fingerprint image
      while (p != FINGERPRINT_OK) {
        
        p = finger.getImage();
        if (p == FINGERPRINT_NOFINGER) {
//          callbackFunc("fingerprint_nofinger", enrollmentMode);
          Serial.print(".");
        } else if (p == FINGERPRINT_OK) {
          callbackFunc("fingerprint_image_taken");
          Serial.println("Image taken");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
          callbackFunc("fingerprint_packetrecieveerr");
          Serial.println("Communication error");
          return p;
        } else if (p == FINGERPRINT_IMAGEFAIL) {
          callbackFunc("fingerprint_imagefail");
          Serial.println("Imaging error");
          return p;
        }
      }

      // Step 2: Convert image to template
      p = finger.image2Tz(1);
      if (p != FINGERPRINT_OK) {
        callbackFunc("fingerprint_converting_error");
        Serial.println("Error converting image");
        return p;
      }

      // Ask user to remove the finger
      Serial.println("Remove finger and place it again...");
      callbackFunc("fingerprint_remove");
      delay(2000);

      // Wait for the user to remove the finger
      while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
      }

      // Step 3: Capture second fingerprint image
      Serial.println("Place the same finger again...");
      callbackFunc("fingerprint_touch_2");
      p = -1;
      while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        if (p == FINGERPRINT_NOFINGER) {
          Serial.print(".");
//          callbackFunc("fingerprint_nofinger");
        } else if (p == FINGERPRINT_OK) {
          Serial.println("Image taken");
          callbackFunc("fingerprint_image_taken");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
          Serial.println("Communication error");
          callbackFunc("fingerprint_image_taken");
          return p;
        } else if (p == FINGERPRINT_IMAGEFAIL) {
          Serial.println("Imaging error");
          callbackFunc("fingerprint_imagefail");
          return p;
        }
      }

      // Step 4: Convert second image to template
      p = finger.image2Tz(2);
      if (p != FINGERPRINT_OK) {
        Serial.println("Error converting image");
        callbackFunc("fingerprint_error_convert_image");
        return p;
      }

      // Step 5: Create a fingerprint model
      p = finger.createModel();
      if (p == FINGERPRINT_OK) {
        Serial.println("Fingerprints matched!");
        callbackFunc("fingerprint_matched");
      } else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        callbackFunc("fingerprint_error_mismatch");
        return p;
      } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        callbackFunc("fingerprint_packetrecieveerr");
        return p;
      } else {
        Serial.println("Unknown error");
        callbackFunc("fingerprint_unknow_error");
        return p;
      }

      // Step 6: Store the fingerprint model in the new ID slot
      p = finger.storeModel(id);
      if (p == FINGERPRINT_OK) {
        App::enrollFingerprint(employeeId, id);

        enrollmentMode = false;
        Serial.println("Fingerprint stored successfully!");
        callbackFunc("fingerprint_stored");
      } else if (p == FINGERPRINT_BADLOCATION) {
        callbackFunc("fingerprint_badlocation");
        Serial.println("Could not store fingerprint in that location");
        return p;
      } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        callbackFunc("fingerprint_flasherr");
        return p;
      } else {
        Serial.println("Unknown error");
        callbackFunc("fingerprint_unknow_error");
        return p;
      }

      return p;
    }

    uint8_t getFingerprintID() {
      uint8_t p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
          Serial.println("Image taken");
          break;
        case FINGERPRINT_NOFINGER:
          Serial.println("No finger detected");
          return p;
        case FINGERPRINT_PACKETRECIEVEERR:
          Serial.println("Communication error");
          return p;
        case FINGERPRINT_IMAGEFAIL:
          Serial.println("Imaging error");
          return p;
        default:
          Serial.println("Unknown error");
          return p;
      }

      // OK success!

      p = finger.image2Tz();
      switch (p) {
        case FINGERPRINT_OK:
          Serial.println("Image converted");
          break;
        case FINGERPRINT_IMAGEMESS:
          Serial.println("Image too messy");
          return p;
        case FINGERPRINT_PACKETRECIEVEERR:
          Serial.println("Communication error");
          return p;
        case FINGERPRINT_FEATUREFAIL:
          Serial.println("Could not find fingerprint features");
          return p;
        case FINGERPRINT_INVALIDIMAGE:
          Serial.println("Could not find fingerprint features");
          return p;
        default:
          Serial.println("Unknown error");
          return p;
      }

      // OK converted!
      p = finger.fingerSearch();
      if (p == FINGERPRINT_OK) {
        Serial.println("Found a print match!");
      } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
      } else if (p == FINGERPRINT_NOTFOUND) {
        Serial.println("Did not find a match");
        return p;
      } else {
        Serial.println("Unknown error");
        return p;
      }

      // found a match!
      Serial.print("Found ID #"); Serial.print(finger.fingerID);
      Serial.print(" with confidence of "); Serial.println(finger.confidence);
      App::checkin(finger.fingerID);

      return finger.fingerID;
    }

};
FingerprintCallback Fingerprint::callbackFunc = nullptr;
