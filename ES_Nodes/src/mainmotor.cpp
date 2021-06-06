#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <MotorDriver.h>
#include <ArduinoJson.h>

const String deviceId = "73aecc58-9e23-44a6-bfd0-0ef2af38435c";
const char *websockets_server_host = "10.10.1.1"; //Enter server adress
const uint16_t websockets_server_port = 80;       // Enter server port

static const int ledIndicatorPin = D8;

// Store motor state
bool motorOpen = true;

const char *ssid = "ES_Master";            //Enter SSID
const char *password = "5vS4hFyM3fEfFvYt"; //Enter Password

String serverUpstreamAddress = "http://10.10.1.1";
websockets::WebsocketsClient client;

// Timer
unsigned long currentTime;
unsigned long prevTimeConnCheck = 0;
const unsigned long intervalConnCheck = 5000;
int failCounter = 0;

// Websocket shit
bool wsReplied;
void connectToWs()
{
  if (client.connect(websockets_server_host, websockets_server_port, "/ws"))
  {
    Serial.println("WS Connected!");
  }
}

String createMotorResponseJson(String type, String message)
{
  StaticJsonDocument<300> doc;
  String motorStat = motorOpen ? "open" : "closed";
  doc["deviceId"] = deviceId;
  doc["type"] = type;
  doc["rssi"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP();
  doc["message"] = message;
  doc["motorStatus"] = motorStat;

  // Serial.printf("\nMotor status: %s\n", motorStat.c_str());

  String out;
  serializeJson(doc, out);

  return out;
}

void sendJson(String message)
{
  client.send(message);
}

void wsOnMessageCallback(websockets::WebsocketsMessage message)
{
  wsReplied = true;
  StaticJsonDocument<128> doc;
  Serial.println("Receiving message!");
  DeserializationError err = deserializeJson(doc, message.data());
  if (err)
  {
    Serial.printf("\n[WS Non-JSON Message] (%s)\n", message.data().c_str());
  }
  else
  {
    Serial.printf("\n[WS JSON Message] (%s)\n", message.data().c_str());
    if (doc["type"] == "motor-command")
    {
      MotorDriver::sendCommand(doc["command"].as<char *>(), &motorOpen);
    }
    else if (doc["type"] == "motor-check")
    {
      sendJson(createMotorResponseJson("motor-check", "Alive"));
    }
    else
      Serial.println("Unrecognized command");
  }
}

void wsOnEventsCallback(websockets::WebsocketsEvent event, String data)
{
  if (event == websockets::WebsocketsEvent::ConnectionOpened)
  {
    Serial.println("\nConnnection Opened");
  }
  else if (event == websockets::WebsocketsEvent::ConnectionClosed)
  {
    Serial.println("\nConnnection Closed");
  }
  else if (event == websockets::WebsocketsEvent::GotPing)
  {
    Serial.println("Got a Ping!");
  }
  else if (event == websockets::WebsocketsEvent::GotPong)
  {
    Serial.println("Got a Pong!");
  }
}
void initWsClient()
{
  // run callback when messages are received
  client.onMessage(wsOnMessageCallback);

  // run callback when events are occuring
  client.onEvent(wsOnEventsCallback);

  connectToWs();
}

String createIntroJson()
{
  StaticJsonDocument<128> doc;
  doc["deviceId"] = deviceId;
  doc["type"] = "motor-register";
  doc["rssi"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP();

  String out;
  serializeJson(doc, out);

  return out;
}

void toggleMotor()
{
  if (motorOpen)
  {
    MotorDriver::sendCommand("close", &motorOpen);
  }
  else
  {
    MotorDriver::sendCommand("open", &motorOpen);
  }
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

    if (first)
      initWsClient();
    else
      connectToWs();

    // Reset fail point
    failCounter = 0;
    String introData = createIntroJson();
    sendJson(introData);
    Serial.println("");
    Serial.println(introData);
    wsReplied = true;
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  MotorDriver::initPin(130);

  pinMode(ledIndicatorPin, OUTPUT);
  digitalWrite(ledIndicatorPin, motorOpen ? HIGH : LOW);

  reconnect(true);
  wsReplied = true;

  // for (;;)
  // {
  //   MotorDriver::setMotorRunning(1, 0, 1, 0);
  //   delay(3500);
  //   MotorDriver::setMotorRunning(0, 1, 0, 1);
  //   delay(500);
  // }
}

void loop()
{
  currentTime = millis();

  // Always check for connection
  if (currentTime - prevTimeConnCheck > intervalConnCheck)
  {
    Serial.println("\nChecking for connection.");
    prevTimeConnCheck = currentTime;
    // toggleMotor();
    if (failCounter > 2)
    {
      Serial.println("No reply");
      reconnect(false, true);
      // ESP.reset();
      return;
    }
    if (!wsReplied)
    {
      failCounter++;
    }
    else
    {
      failCounter = 0;
      wsReplied = false;
      sendJson(createMotorResponseJson("motor-check", "Ok"));
    }
    reconnect();
  }

  client.poll();
}