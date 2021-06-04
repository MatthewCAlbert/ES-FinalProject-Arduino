#include <Arduino.h>
#include <env.h>
#include "ESP8266WiFi.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>
#include <ws.h>

const String deviceId = "8218e23c-80cc-49b4-9f6c-be25aee3d6f3";

// Timer
unsigned long prevTimeConnCheck = 0;
unsigned long prevTimeSend = 0;
unsigned long currentTime;
unsigned long intervalConnCheck = 10000;
unsigned long intervalSend = 3000;

int failCounter = 0;

// Multiplexer
int MUXPinS0 = D5;
int MUXPinS1 = D6;
int MUXPinS2 = D7;
int MUXPinS3 = D8;

// Temp Sensor
#define DHTTYPE DHT11
#define TEMPSENSOR_PIN 2
#define LDR_PIN 0
DHT_Unified dht(TEMPSENSOR_PIN, DHTTYPE);

// Rain Drop Sensor
#define RAINDROPSENSOR_A_PIN 1
#define RAINDROPSENSOR_D_PIN 5

float round(float var)
{
  float value = (int)(var * 100 + .5);
  return (float)value / 100;
}

String createIntroJson()
{
  StaticJsonDocument<128> doc;
  doc["deviceId"] = deviceId;
  doc["type"] = "sensor-register";
  doc["rssi"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP();

  String out;
  serializeJson(doc, out);

  return out;
}

void reconnect(bool first = false, bool force = false)
{
  if (WiFi.status() != WL_CONNECTED || force)
  {
    //Attempt connect again
    if (first)
      Serial.print("\nConnecting to WiFi");
    else
    {
      Serial.print("\nReconnecting to WiFi");
      WS::disconnect();
      WiFi.disconnect();
    }
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connection Successful");
    Serial.print("The IP Address of ESP8266 Sensor Node is: ");
    Serial.print(WiFi.localIP()); // Print the IP address

    if (first)
      WS::initWsClient();
    else
      WS::connectToWs();

    // Reset fail point
    failCounter = 0;
    WS::setReplied(true);
    String introData = createIntroJson();
    WS::sendJson(introData);
    Serial.println("");
    Serial.println(introData);
  }
}

String createSerializedJsonSensorPack(float wetness, float temperature, float humidity, float luminance)
{
  StaticJsonDocument<300> doc;
  doc["deviceId"] = deviceId;
  doc["type"] = "sensor-data";
  doc["rssi"] = WiFi.RSSI();

  JsonObject obj = doc.createNestedObject("data");
  obj["wetness"] = wetness;
  obj["temperature"] = temperature;
  obj["humidity"] = humidity;
  obj["luminance"] = luminance;

  String out;
  serializeJson(doc, out);

  return out;
}

float getAnalog(int MUXyPin)
{
  //MUXyPin must be 0 to 15 representing the analog pin you want to read
  //MUXPinS3 to MUXPinS0 are the Arduino pins connected to the selector pins of this board.
  digitalWrite(MUXPinS3, HIGH && (MUXyPin & B00001000));
  digitalWrite(MUXPinS2, HIGH && (MUXyPin & B00000100));
  digitalWrite(MUXPinS1, HIGH && (MUXyPin & B00000010));
  digitalWrite(MUXPinS0, HIGH && (MUXyPin & B00000001));
  return (float)analogRead(A0);
}

void printDht()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
}

float getTemperature()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
  {
    return event.temperature;
  }
  else
  {
    return 0;
  }
}

float getHumidity()
{
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    return event.relative_humidity;
  }
  else
  {
    return 0;
  }
}

float getLuminance()
{
  return getAnalog(LDR_PIN);
}

float getRainWetness()
{
  return (1024.0 - getAnalog(RAINDROPSENSOR_A_PIN)) / 1024.0;
}

void printDebugAllSensor()
{
  float rainsensor = getRainWetness();
  float percentmodifier = 100.0;
  float rainpercent = rainsensor * percentmodifier;
  Serial.printf("\nRain Reading Wetness: %.4f (%.2f\%)\n", rainsensor, rainpercent);
  Serial.printf("\nLDR Reading Value: %.2f\n", getAnalog(LDR_PIN));
  printDht();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Multiplexer
  pinMode(MUXPinS0, OUTPUT);
  pinMode(MUXPinS1, OUTPUT);
  pinMode(MUXPinS2, OUTPUT);
  pinMode(MUXPinS3, OUTPUT);

  // Other sensor
  dht.begin();
  pinMode(RAINDROPSENSOR_D_PIN, INPUT);
  reconnect(true);
  WS::setReplied(true);
}

void loop()
{
  currentTime = millis();

  // Always check for connection
  if (currentTime - prevTimeConnCheck > intervalConnCheck)
  {
    Serial.println("\nChecking for connection.");
    Serial.println(WiFi.status());
    prevTimeConnCheck = currentTime;
    if (!WS::getWsHealth())
    {
      Serial.println("Something wrong reconnecting.");
      reconnect();
    }
    else if (failCounter > 2)
    {
      ESP.restart();
    }
    else
    {
      Serial.println("OK");
    }
  }

  if (currentTime - prevTimeSend > intervalSend)
  {
    prevTimeSend = currentTime;
    String jsonPackage = createSerializedJsonSensorPack(round(getRainWetness()), getTemperature(), getHumidity(), getLuminance());

    if (WS::getReplied() && WS::sendJson(jsonPackage))
    {
      WS::setReplied(false);
      Serial.println("Message Sent");
    }
    else
    {
      failCounter++;
      Serial.println("Message Fail to Send");
    }

    Serial.println("");
    Serial.print(jsonPackage);
  }

  WS::poll();
}