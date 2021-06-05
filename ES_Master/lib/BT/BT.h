#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial ESP_BT;
int incoming;

void initBt()
{
  ESP_BT.begin("ES_Master");
  Serial.println("Bluetooth Device is Ready to Pair");
}

void btLoop()
{
  if (ESP_BT.available()) //Check if we receive anything from Bluetooth
  {
    incoming = ESP_BT.read(); //Read what we recevive
    Serial.print("Received:");
    Serial.println(incoming);

    if (incoming == 49)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      ESP_BT.println("LED turned ON");
    }

    if (incoming == 48)
    {
      digitalWrite(LED_BUILTIN, LOW);
      ESP_BT.println("LED turned OFF");
    }
  }
}