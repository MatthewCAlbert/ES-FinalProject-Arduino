#include "oled.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void OLED::initOled()
{
  // Init SSD1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Hello, world!");
  display.display();
}

void OLED::updateConnectedStationDisplay(void *parameter)
{
  for (;;)
  {
    display.clearDisplay();
    display.setCursor(10, 20);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    int connectedDevice = WiFi.softAPgetStationNum();
    Serial.printf("\nConnected: %d", connectedDevice);
    display.printf("Connected: %d", connectedDevice);
    display.display();
    delay(3000);
  }
}