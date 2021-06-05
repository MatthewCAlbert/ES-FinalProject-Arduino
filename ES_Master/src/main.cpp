#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <utils.h>
#include <esWifi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
// #include <BT.h>
// #include <BLE.h>

// Don't forget to create this file first
const char *deviceId = "f17d53fd-f843-431b-a5e0-305126625841";
#include "secret.h"

const bool ENABLE_HTTP_ENDPOINT = true;
const bool DISABLE_STAMODE = false;

// WiFi AP Credential
const char *ap_ssid = "ES_Master";
const char *ap_password = "5vS4hFyM3fEfFvYt";
const int ap_channel = 9;
const int hide_ssid = 0;
const int ap_max_connection = 3;
IPAddress IP = {10, 10, 1, 1};
IPAddress gateway = {10, 10, 1, 1};
IPAddress NMask = {255, 255, 255, 248};
bool firstWifiSTAConnectInitiated = false;

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define RST_BTN 32

static const int LED1_PIN = 4;
static const int LED2_PIN = 16;
static const int LED3_PIN = 17;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t TaskWifi;
TaskHandle_t TaskMqtt;
TaskHandle_t TaskMqttTx;
TaskHandle_t TaskMakeDecision;
static HTTPClient http;

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

void decisionLoop(void *parameter)
{
  for (;;)
  {
    ESWifi::decisionMaker();
    delay(5000);
  }
}

void mqttReconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESMasterClient-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("\n[MQTT] Connected");
      String subTopic = "escommand-" + (String)deviceId;

      Serial.printf("[MQTT] Subscribe to command %s\n", mqttClient.subscribe("es/command") ? "SUCCESS" : "FAILED");
      Serial.printf("[MQTT] Subscribe to bmkg %s\n", mqttClient.subscribe("es/bmkg") ? "SUCCESS" : "FAILED");
    }
    else
    {
      Serial.println("[MQTT] Connect Failed");
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void checkMqttConnection(void *parameter)
{
  for (;;)
  {
    // Serial.println("\n[MQTT] Checking connection");
    setLedIndicator();

    if (WiFi.status() == WL_CONNECTED)
      mqttReconnect();
    delay(1000);
  }
}

void sendMqttPublishData()
{
  if (mqttClient.connected())
  {
    Serial.println("\n[MQTT] Publishing data");
    // String pubTopic = "esdata-" + (String)deviceId;
    String payload = getMqttSendPacketData();
    // Serial.println(payload);

    if (mqttClient.publish("es/data", payload.c_str()))
    {
      Serial.println("[MQTT] Publishing data OK");
    }
    else
    {
      Serial.println("[MQTT] Publish data FAILED");
    }
  }
}

void mqttIntervalTxAction(void *parameter)
{
  for (;;)
  {
    sendMqttPublishData();
    delay(30000);
  }
}

void reconnect(bool first = false, bool force = false)
{
  //Attempt connect again
  if (WiFi.status() != WL_CONNECTED)
  {
    if (first)
    {
      Serial.print("\nConnecting to Upstream WiFi");
    }
    else
    {
      Serial.print("\nReconnecting to Upstream WiFi");
      WiFi.disconnect();
    }
    WiFi.begin(m_ssid, m_password);
    int dotCounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      dotCounter++;
      if (dotCounter > 30)
      {
        WiFi.disconnect();
        return;
      }
    }

    Serial.println("");
    Serial.println("Upstream WiFi connection Successful");
    Serial.print("The IP Address of ESP32 Module is: ");
    Serial.print(WiFi.localIP()); // Print the IP address

    if (first)
    {
      initNtp();
      mqttClient.setServer(mqtt_server, 1883);
      mqttClient.setCallback(MQTTClient::callback);
      mqttClient.setBufferSize(4096);
      firstWifiSTAConnectInitiated = true;
    }
  }
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

void staCheckLoop(void *parameter)
{
  for (;;)
  {
    Serial.println("Checking WiFi Connection..");
    if (!firstWifiSTAConnectInitiated)
      reconnect(true);
    else
      reconnect();
    delay(30000);
  }
}

void checkMotorAvailable(void *parameter)
{
  for (;;)
  {
    ESWifi::pingMotor();
    delay(10000);
  }
}

void displayNtpClock(void *parameter)
{
  for (;;)
  {
    Serial.println(getTimestamp());
    delay(5000);
  }
}

void loopBTTest(void *parameter)
{
  for (;;)
  {
    // btLoop();
    // delay(20);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  // initI2C();

  // PIN Setup
  Serial.println("Hello ESP32 Started!");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(RST_BTN, INPUT_PULLDOWN);

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  // Clear WiFi previous saved config to prevent AP problem
  WiFi.disconnect(true, true);

  if (!DISABLE_STAMODE)
  {
    WiFi.mode(WIFI_MODE_APSTA);
    xTaskCreatePinnedToCore(staCheckLoop, "Wifi STA Check Loop", 10000, NULL, 0, &TaskWifi, 1);
  }
  else
  {
    WiFi.mode(WIFI_MODE_AP);
  }
  // Create AP
  WiFi.softAP(ap_ssid, ap_password, ap_channel, hide_ssid, ap_max_connection);
  WiFi.softAPConfig(IP, gateway, NMask);
  WiFi.onEvent(ESWifi::WiFiStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
  WiFi.onEvent(ESWifi::WiFiStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Start Server
  WebSocketServer::initWebSocket();

  // Start Web HTTP Route
  if (ENABLE_HTTP_ENDPOINT)
    WebSocketServer::initWebRoute();

  // Init Bluetooth
  // initBLE();
  // initBt();

  // Create Multitask
  xTaskCreatePinnedToCore(checkMotorAvailable, "Check Motor Availabiliy Status", 10000, NULL, 0, &Task1, 1);
  xTaskCreatePinnedToCore(decisionLoop, "Make Decision", 10000, NULL, 0, &TaskMakeDecision, 1);
  xTaskCreatePinnedToCore(checkMqttConnection, "Check MQTT Status", 10000, NULL, 1, &TaskMqtt, 1);
  xTaskCreatePinnedToCore(mqttIntervalTxAction, "MQTT Interval TX", 30000, NULL, 2, &TaskMqttTx, 1);

  // Multitask for testing
  // xTaskCreatePinnedToCore(loopBTTest, "BT Test Loop", 10000, NULL, 0, &Task2, 1);
  // xTaskCreatePinnedToCore(displayNtpClock, "Check Time", 10000, NULL, 0, &Task2, 1);
}

void loop()
{
  currentTime_ms = millis();
  // put your main code here, to run repeatedly:
  readRstButton();
  // checkWiFiConnection(); // No longer used, task moved to thread
  // checkRF();
  WebSocketServer::wsLoop();

  if (mqttClient.connected())
    mqttClient.loop();
}