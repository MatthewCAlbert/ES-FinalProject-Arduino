#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <utils.h>
// #include <rf.h>
#include <oled.h>
#include <esWifi.h>
#include <HTTPClient.h>

// Don't forget to create this file first
#include "secret.h"

// WiFi AP Credential
const char *ap_ssid = "ES_Master";
const char *ap_password = "5vS4hFyM3fEfFvYt";
const int ap_channel = 9;
const int hide_ssid = 1;
const int ap_max_connection = 3;
IPAddress IP = {10, 10, 1, 1};
IPAddress gateway = {10, 10, 1, 1};
IPAddress NMask = {255, 255, 255, 248};

#define RST_BTN 32

TaskHandle_t Task1;
TaskHandle_t Task2;

unsigned long currentTime_ms;
const unsigned long wifiCheckInterval_ms = 10000;
const unsigned long resetButtonTimeout_ms = 5000;
unsigned long lastDebounceTime = 0;
unsigned long lastWifiCheckTime = 0;
byte prevKeyState = LOW;
void readRstButton()
{
  if (digitalRead(RST_BTN) == HIGH && prevKeyState == LOW)
  {
    prevKeyState = HIGH;
    lastDebounceTime = currentTime_ms;
  }
  else if (digitalRead(RST_BTN) == LOW && prevKeyState == HIGH)
  {
    prevKeyState = LOW;
    if (currentTime_ms - lastDebounceTime > resetButtonTimeout_ms)
      Serial.println("Hi from button youve pressed more than 5 sec");
    else
      Serial.println("Pressed once");
  }
  delay(20);
}

void reconnect(bool first = false)
{
  //Attempt connect again
  if (first)
    Serial.print("\nConnecting to Upstream WiFi");
  else
  {
    Serial.print("\nReconnecting to Upstrea m WiFi");
    WiFi.disconnect();
  }
  WiFi.begin(m_ssid, m_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Upstream WiFi connection Successful");
  Serial.print("The IP Address of ESP32 Module is: ");
  Serial.print(WiFi.localIP()); // Print the IP address
}

void checkWiFiConnection()
{
  if (currentTime_ms - lastWifiCheckTime > wifiCheckInterval_ms)
  {
    lastWifiCheckTime = currentTime_ms;
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.disconnect();
      reconnect();
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  initI2C();

  // Init OLED
  // OLED::initOled();

  // PIN Setup
  Serial.println("Hello ESP32 Started!");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(RST_BTN, INPUT_PULLDOWN);

  WiFi.mode(WIFI_MODE_APSTA);
  // Create AP
  WiFi.softAP(ap_ssid, ap_password, ap_channel, hide_ssid, ap_max_connection);
  WiFi.softAPConfig(IP, gateway, NMask);
  WiFi.onEvent(ESWifi::WiFiStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
  WiFi.onEvent(ESWifi::WiFiStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  reconnect(true);

  // Start Server
  WebSocketServer::initWebSocket();
  WebSocketServer::initWebRoute();

  // Create Multitask
  // xTaskCreatePinnedToCore(OLED::updateConnectedStationDisplay, "Station Connected Looping", 10000, NULL, 0, &Task1, 1);
}

void loop()
{
  currentTime_ms = millis();
  // put your main code here, to run repeatedly:
  readRstButton();
  checkWiFiConnection();
  // checkRF();
  WebSocketServer::wsLoop();
}