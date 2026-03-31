#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- BLE CONFIGURATION ---
// These UUIDs identify the specific Food Scanner service
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// --- HARDWARE PIN ASSIGNMENTS (XIAO ESP32-S3) ---
const int redPin = 2;    // D1
const int greenPin = 3;  // D2
const int sensorPin = 1; // D0 (Analog Input)
const int threshold = 2500; 

// Callback class to monitor connection status
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Phone Connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Phone Disconnected!");
      // Restart advertising so it can be found again
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // 1. Initialize BLE Device
  BLEDevice::init("ChompSafe-Scanner");

  // 2. Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. Create the BLE Characteristic (Read + Notify)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Add a descriptor so the phone knows it can subscribe to notifications
  pCharacteristic->addDescriptor(new BLE2902());

  // 5. Start the service
  pService->start();

  // 6. Start Advertising so your S23 FE can see it
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->start();

  Serial.println("BLE Active. Waiting for phone...");
}

void loop() {
  // We simulate the "Scan" trigger. 
  // In BLE, you can either poll the sensor or wait for a phone command.
  // Here, we'll scan whenever light is detected.
  
  int lightLevel = analogRead(sensorPin);

  if (lightLevel > threshold) {
    Serial.print("Scan Triggered! Value: ");
    Serial.println(lightLevel);

    // Perform logic
    String result = (random(0, 2) == 0) ? "RED" : "GREEN";

    // 1. Update BLE Characteristic
    if (deviceConnected) {
      pCharacteristic->setValue(result.c_str());
      pCharacteristic->notify(); // Push the "RED" or "GREEN" text to the phone
      Serial.println("Result sent via BLE: " + result);
    }

    // 2. Visual Feedback (Blinking)
    int activePin = (result == "RED") ? redPin : greenPin;
    for (int i = 0; i < 10; i++) {
      digitalWrite(activePin, HIGH); delay(100);
      digitalWrite(activePin, LOW);  delay(100);
    }
    
    delay(1000); // Cooldown to prevent double-scans
  }
}