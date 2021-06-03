#include <Arduino.h>
#include <Wire.h>

#define LED_1 23

void ledLoop(void *parameter)
{
  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(LED_1, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_1, LOW);
    delay(1000);
  }
}

void initI2C()
{
  Wire.begin(17, 22);
}

void i2cScannerLoop(void *parameter)
{
  for (;;)
  {
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
      {
        Serial.print("I2C device found at address 0x");
        if (address < 16)
        {
          Serial.print("0");
        }
        Serial.println(address, HEX);
        nDevices++;
      }
      else if (error == 4)
      {
        Serial.print("Unknow error at address 0x");
        if (address < 16)
        {
          Serial.print("0");
        }
        Serial.println(address, HEX);
      }
    }
    if (nDevices == 0)
    {
      Serial.println("No I2C devices found\n");
    }
    else
    {
      Serial.println("done\n");
    }
    delay(5000);
  }
}