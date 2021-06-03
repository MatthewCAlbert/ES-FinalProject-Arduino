#include <Arduino.h>
#include "ESP8266WiFi.h"

const char *ssid = "ES_Master";            //Enter SSID
const char *password = "5vS4hFyM3fEfFvYt"; //Enter Password

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }

  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("The IP Address of ESP8266 Module is: ");
  Serial.print(WiFi.localIP()); // Print the IP address
}

void loop()
{
  // put your main code here, to run repeatedly:
}