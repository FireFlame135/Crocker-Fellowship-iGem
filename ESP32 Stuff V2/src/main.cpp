#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Preferences.h>

// --- BLE UUID REGISTRY (ALL NEW UUIDS) ---
#define CHOMPSAFE_SERVICE_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f301"
#define DEVICE_INFO_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f302"
#define COMMAND_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f303"
#define STATUS_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f304"
#define RESULT_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f305"
#define CONFIG_UUID "f0e8c930-6f61-4f8a-9f38-c224ce75f306"

// --- HARDWARE CONFIGURATION (XIAO ESP32-S3) ---
const int RED_PIN = 2;     // D1
const int GREEN_PIN = 3;   // D2
const int SENSOR_PIN = 1;  // D0

const int DEFAULT_THRESHOLD = 700;
const int MIN_THRESHOLD = 500;
const int MAX_THRESHOLD = 900;
const int SAMPLE_COUNT = 8;

Preferences preferences;

BLEServer* g_server = nullptr;
BLECharacteristic* g_deviceInfoCharacteristic = nullptr;
BLECharacteristic* g_commandCharacteristic = nullptr;
BLECharacteristic* g_statusCharacteristic = nullptr;
BLECharacteristic* g_resultCharacteristic = nullptr;
BLECharacteristic* g_configCharacteristic = nullptr;

bool g_deviceConnected = false;
bool g_pendingScan = false;
int g_currentThreshold = DEFAULT_THRESHOLD;

void notifyCharacteristic(BLECharacteristic* characteristic, const String& payload) {
  if (characteristic == nullptr) {
    return;
  }
  characteristic->setValue(payload.c_str());
  if (g_deviceConnected) {
    characteristic->notify();
  }
}

String buildStatusPayload(const String& state, const String& errorMessage = "") {
  JsonDocument doc;
  doc["state"] = state;
  doc["error"] = errorMessage;
  doc["ts"] = millis();

  String payload;
  serializeJson(doc, payload);
  return payload;
}

String buildConfigPayload() {
  JsonDocument doc;
  doc["threshold"] = g_currentThreshold;
  doc["profiles"]["mostSafe"] = 550;
  doc["profiles"]["moreSafe"] = 625;
  doc["profiles"]["normal"] = 700;
  doc["profiles"]["lessSafe"] = 775;
  doc["profiles"]["leastSafe"] = 850;

  String payload;
  serializeJson(doc, payload);
  return payload;
}

void publishStatus(const String& state, const String& errorMessage = "") {
  const String payload = buildStatusPayload(state, errorMessage);
  notifyCharacteristic(g_statusCharacteristic, payload);
  Serial.println("STATUS -> " + payload);
}

void refreshConfigCharacteristic() {
  if (g_configCharacteristic == nullptr) {
    return;
  }
  g_configCharacteristic->setValue(buildConfigPayload().c_str());
}

void saveThreshold(int threshold) {
  preferences.putInt("threshold", threshold);
}

int readAveragedSensorValue() {
  long total = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(SENSOR_PIN);
    delay(4);
  }
  return static_cast<int>(total / SAMPLE_COUNT);
}

void blinkOutcome(bool isUnsafe) {
  const int activePin = isUnsafe ? RED_PIN : GREEN_PIN;
  for (int i = 0; i < 5; i++) {
    digitalWrite(activePin, HIGH);
    delay(70);
    digitalWrite(activePin, LOW);
    delay(60);
  }
}

void processScan() {
  publishStatus("SCANNING");

  const int rawSensorValue = readAveragedSensorValue();
  const bool isUnsafe = rawSensorValue > g_currentThreshold;

  JsonDocument doc;
  doc["raw"] = rawSensorValue;
  doc["threshold"] = g_currentThreshold;
  doc["ts"] = millis();

  String payload;
  serializeJson(doc, payload);
  notifyCharacteristic(g_resultCharacteristic, payload);
  Serial.println("RESULT -> " + payload);

  blinkOutcome(isUnsafe);
  publishStatus("RESULT_READY");
  delay(20);
  publishStatus("IDLE");
}

class ChompSafeServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    g_deviceConnected = true;
    Serial.println("Phone connected");
    publishStatus("IDLE");
    refreshConfigCharacteristic();
  }

  void onDisconnect(BLEServer* server) override {
    g_deviceConnected = false;
    Serial.println("Phone disconnected");
    server->getAdvertising()->start();
  }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    const String value = String(characteristic->getValue().c_str());
    if (value.length() == 0) {
      publishStatus("ERROR", "Empty command payload");
      return;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, value);
    if (error) {
      publishStatus("ERROR", "Invalid command JSON");
      return;
    }

    const String command = String(doc["cmd"] | "");
    if (command == "SCAN") {
      g_pendingScan = true;
      return;
    }

    if (command == "GET_CONFIG") {
      refreshConfigCharacteristic();
      publishStatus("IDLE");
      return;
    }

    publishStatus("ERROR", "Unsupported command");
  }
};

class ConfigCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* characteristic) override {
    refreshConfigCharacteristic();
  }

  void onWrite(BLECharacteristic* characteristic) override {
    const String value = String(characteristic->getValue().c_str());
    if (value.length() == 0) {
      publishStatus("ERROR", "Empty config payload");
      refreshConfigCharacteristic();
      return;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, value);
    if (error) {
      publishStatus("ERROR", "Invalid config JSON");
      refreshConfigCharacteristic();
      return;
    }

    if (!doc["threshold"].is<int>()) {
      publishStatus("ERROR", "Missing threshold");
      refreshConfigCharacteristic();
      return;
    }

    int requestedThreshold = doc["threshold"].as<int>();
    if (requestedThreshold < MIN_THRESHOLD || requestedThreshold > MAX_THRESHOLD) {
      publishStatus("ERROR", "Threshold out of range");
    }

    requestedThreshold = constrain(requestedThreshold, MIN_THRESHOLD, MAX_THRESHOLD);
    g_currentThreshold = requestedThreshold;
    saveThreshold(g_currentThreshold);

    Serial.print("Threshold updated: ");
    Serial.println(g_currentThreshold);

    refreshConfigCharacteristic();
    publishStatus("IDLE");
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  preferences.begin("chompsafe", false);
  g_currentThreshold = preferences.getInt("threshold", DEFAULT_THRESHOLD);
  g_currentThreshold = constrain(g_currentThreshold, MIN_THRESHOLD, MAX_THRESHOLD);

  BLEDevice::init("ChompSafe-Scanner");
  g_server = BLEDevice::createServer();
  g_server->setCallbacks(new ChompSafeServerCallbacks());

  BLEService* service = g_server->createService(CHOMPSAFE_SERVICE_UUID);

  g_deviceInfoCharacteristic = service->createCharacteristic(
      DEVICE_INFO_UUID,
      BLECharacteristic::PROPERTY_READ);
  g_deviceInfoCharacteristic->setValue(
      "{\"fw\":\"1.0.0\",\"hw\":\"xiao-esp32s3\",\"name\":\"ChompSafe-Scanner\"}");

  g_commandCharacteristic = service->createCharacteristic(
      COMMAND_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
  g_commandCharacteristic->setCallbacks(new CommandCallbacks());
  g_commandCharacteristic->setValue("{\"cmd\":\"IDLE\"}");

  g_statusCharacteristic = service->createCharacteristic(
      STATUS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  g_statusCharacteristic->addDescriptor(new BLE2902());

  g_resultCharacteristic = service->createCharacteristic(
      RESULT_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  g_resultCharacteristic->addDescriptor(new BLE2902());
  g_resultCharacteristic->setValue("{\"raw\":0,\"threshold\":700,\"ts\":0}");

  g_configCharacteristic = service->createCharacteristic(
      CONFIG_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  g_configCharacteristic->setCallbacks(new ConfigCallbacks());
  refreshConfigCharacteristic();

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(CHOMPSAFE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->start();

  Serial.println("BLE active. Waiting for app...");
  publishStatus("IDLE");
}

void loop() {
  if (!g_pendingScan) {
    delay(20);
    return;
  }

  g_pendingScan = false;
  processScan();
}