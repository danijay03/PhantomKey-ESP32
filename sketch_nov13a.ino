//void setup() {
  // put your setup code here, to run once:
//}

//void loop() {
  // put your main code here, to run repeatedly:
//}

#include <NimBLEDevice.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE...");

  // Initialize BLE with device name
  NimBLEDevice::init("PhantomKey");
  NimBLEServer *pServer = NimBLEDevice::createServer();

  // Create a basic service
  NimBLEService *pService = pServer->createService("12345678-1234-1234-1234-1234567890ab");

  // Add a characteristic
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
    "abcdefab-1234-5678-9abc-def123456789",
    NIMBLE_PROPERTY::READ
  );
  pCharacteristic->setValue("Hello from ESP32!");

  // Start service
  pService->start();

  // Start advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());

  // ✅ Include the device name in advertisement
//  pAdvertising->setScanResponseData(); // Include additional info in scan response
  pAdvertising->setName("PhantomKey"); // Explicitly set name

  NimBLEDevice::startAdvertising();
  Serial.println("BLE Advertising as 'PhantomKey'...");
}

void loop() {
  delay(1000);
}

//void setup() {
  //Serial.begin(115200);
  //delay(1000);
  //Serial.println("ESP32 is alive!");
//}

//void loop() {
  //Serial.println("Running...");
  //delay(1000);
//}

