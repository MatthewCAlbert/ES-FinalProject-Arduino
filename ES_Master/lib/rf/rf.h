#include <Arduino.h>
#include <SPI.h>
#include <RH_NRF24.h>

#define CE_PIN 15
#define CSN_PIN 13

class NRF
{
public:
  static void initRf();
  static void checkRF();
};

// You can choose any of several encryption ciphers
// Speck myCipher; // Instantiate a Speck block ciphering
// The RHEncryptedDriver acts as a wrapper for the actual radio driver
// RHEncryptedDriver driver(nrf24, myCipher); // Instantiate the driver with those two
// The key MUST be the same as the one in the client
// unsigned char encryptkey[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};