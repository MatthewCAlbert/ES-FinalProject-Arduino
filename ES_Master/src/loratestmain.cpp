#include <Arduino.h>
#include <AntaresLoRaWAN.h>
#include <EEPROM.h>

#define ACCESSKEY "a070457d79de3f57:c0d0d0df571acd05"
#define DEVICEID "YI_VnzQ9QJ-f0dqs"
#define vbatt 32

AntaresLoRaWAN antares;
size_t EPROM_MEMORY_SIZE = 512;

float nilaivbatt;
String dataSend = "";
int readaddress;
int address = 0;
int hitung;

//Fungsi send data, disarankan untuk tidak merubah fungsi ini
void sendPacket(String &input)
{
  String dataA = String(input);
  antares.send(dataA);

  char dataBuf[50];
  int dataLen = dataA.length();

  dataA.toCharArray(dataBuf, dataLen + 1);
  Serial.println("Data :" + (String)dataLen);
  if (dataLen > 1)
  {
    Serial.println("\n[ANTARES] Data: " + dataA + "\n");

    if (LMIC.opmode & OP_TXRXPEND)
    {
      Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
      LMIC_setTxData2(1, (uint8_t *)dataBuf, dataLen, 0);
      Serial.println(F("Packet queued"));
      esp_sleep_enable_timer_wakeup(10 * 1000000);
      //esp_sleep_enable_ext0_wakeup(GPIO_NUM_34,0);
      esp_deep_sleep_start();
    }
  }
  else
  {
    Serial.println("\n[ANTARES] Data: Kosong\n");
  }
  delay(10000);
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Starting"));
  EEPROM.begin(EPROM_MEMORY_SIZE);
  EEPROM.get(address, readaddress);
  hitung = readaddress;
  antares.setPins(15, 25, 26); // Set pins for NSS, DIO0, and DIO1
  antares.setTxInterval(1);    // Set the amount of interval time (in seconds) to transmit
  antares.setSleep(true);      // antares.setSleep(true, 10);
  antares.init(ACCESSKEY, DEVICEID);
  antares.setDataRateTxPow(DR_SF10, 17);
}

void loop()
{
  hitung = hitung + 1;
  EEPROM.put(address, hitung);
  EEPROM.commit();
  dataSend = "Counting saat ini : " + String(hitung);
  Serial.println(dataSend);
  delay(3000);
  if (hitung >= 4096)
  {
    hitung = 0;
    EEPROM.put(address, hitung);
    EEPROM.commit();
    delay(10);
  }
  if (hitung <= 4095)
  {
    Serial.println("saat ini masih" + String(hitung) + ", belum 4096");
    delay(10);
    sendPacket(dataSend);
  }
}