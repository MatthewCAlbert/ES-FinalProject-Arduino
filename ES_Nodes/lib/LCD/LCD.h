#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void i2cScannerLoop()
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

void initLcd()
{
  lcd.init(); // initialize the lcd
  lcd.backlight();
}

void writeSensorData(float wetness, float max3, float avg15)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Wetness : %.2f", wetness);
  lcd.setCursor(0, 1);
  lcd.printf("M3:%.2f|Avg:%.2f", max3, avg15);
}