#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "85c08e3d-5104-45cc-9b81-4f3317045ed0"
#define CHARACTERISTIC_UUID_TX "a7299232-e488-49c5-9bb9-4c427f01ca0e"
#define CHARACTERISTIC_UUID_RX "f840f659-f73c-4734-b693-932baa8f48a3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      Serial.print("Received Value: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }

      // Serial.println();

      // Do stuff based on the command received from the app
      // if (rxValue.find("A") != -1)
      // {
      //   Serial.print("Turning ON!");
      // }
      // else if (rxValue.find("B") != -1)
      // {
      //   Serial.print("Turning OFF!");
      // }

      Serial.println();
      Serial.println("*********");
    }
  }
};

void initBLE()
{
  BLEDevice::init("ES_Master");

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Service Start
  pService->start();

  // Advertise
  pServer->getAdvertising()->start();
  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  // pAdvertising->addServiceUUID(SERVICE_UUID);
  // pAdvertising->setScanResponse(true);
  // pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  // pAdvertising->setMinPreferred(0x12);
  // BLEDevice::startAdvertising();
}

void bleLoop()
{
  if (deviceConnected)
  {
    pCharacteristic->setValue("Hi");

    // Send the value to the app!
    pCharacteristic->notify();
  }
}