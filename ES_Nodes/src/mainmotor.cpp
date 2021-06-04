#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <MotorDriver.h>
#include <ArduinoJson.h>

const String deviceId = "73aecc58-9e23-44a6-bfd0-0ef2af38435c";

const char *ssid = "ES_Master";            //Enter SSID
const char *password = "5vS4hFyM3fEfFvYt"; //Enter Password

ESP8266WebServer server(80);
String header;

String serverUpstreamAddress = "http://10.10.1.1";

// Timer
unsigned long currentTime;
unsigned long prevTimeConnCheck = 0;
const unsigned long intervalConnCheck = 10000;

bool sendIpAddress()
{
  Serial.println("Sending IP Address..");
  HTTPClient http;
  String serverPath = serverUpstreamAddress + "/motor/updateip";
  http.begin(serverPath.c_str());
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  String postData = "deviceId=" + deviceId + "&ip=" + WiFi.localIP().toString();
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);
  }

  // Free resources
  http.end();

  return (httpResponseCode == 200);
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
    Serial.print("The IP Address of ESP8266 Motor Node is: ");
    Serial.print(WiFi.localIP()); // Print the IP address

    // if (sendIpAddress())
    //   Serial.println("IP Sent!");
    // else
    //   Serial.println("IP Sending Failed!");
  }
}

bool checkConnection()
{
  HTTPClient http;
  String serverPath = serverUpstreamAddress + "/check";
  http.begin(serverPath.c_str());

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    reconnect(false, true);
  }
  // Free resources
  http.end();
  return (httpResponseCode > 0);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  MotorDriver::initPin();

  reconnect(true);

  // initWebRoute();
  // server.begin();
}

void loop()
{
  currentTime = millis();
  // server.handleClient();

  // Always check for connection
  if (currentTime - prevTimeConnCheck > intervalConnCheck)
  {
    Serial.println("\nChecking for connection.");
    prevTimeConnCheck = currentTime;
    MotorDriver::toggle();
    // checkConnection();
  }
}