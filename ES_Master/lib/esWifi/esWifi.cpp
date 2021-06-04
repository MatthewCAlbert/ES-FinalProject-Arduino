#include "esWifi.h"

static HTTPClient http;
static WiFiClient client;

String sensorIp;
uint32_t sensorClientId = 0;

String motorIp = "";
uint32_t motorClientId = 0;
bool motorResponded = false;
String motorStatus = "";

SensorStorage myStorage;

// Websockets
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int connectedDevice = 0;
void ESWifi::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  connectedDevice++;
}
void ESWifi::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  connectedDevice--;
}
void ESWifi::pingMotor()
{
  if (motorIp == "" || motorClientId == 0 || !motorResponded)
  {
    Serial.println("Motor failed to reach.");
    return;
  }
  StaticJsonDocument<128> doc;
  String out;
  doc["type"] = "motor-check";
  serializeJson(doc, out);

  Serial.printf("\n Ping sent to Client #%d\n", motorClientId);
  ws.text(motorClientId, out);
  motorResponded = false;
}
String ESWifi::getCoverStatus()
{
  Serial.printf("%s : #%u \n", motorIp.c_str(), motorClientId);
  if (motorIp == "" || !motorResponded || motorClientId == 0)
  {
    Serial.println("Motor node not connected");
    return "error";
  }
  return motorStatus == "" ? "no status" : motorStatus;
}
bool ESWifi::setCoverStatus(String command)
{
  if (motorIp == "" || !motorResponded || motorClientId == 0)
  {
    Serial.println("Motor node not connected");
    return false;
  }
  StaticJsonDocument<128> doc;
  String out;
  doc["type"] = "motor-command";
  doc["command"] = command;
  serializeJson(doc, out);

  motorResponded = false;
  Serial.printf("\nCommand Sent to Client #%d\n", motorClientId);
  ws.text(motorClientId, out);

  return true;
}

void WebSocketServer::notifyClients()
{
  ws.textAll("ok");
}

// WS Received Data Processor
void WebSocketServer::handleWebSocketMessage(void *arg, AsyncWebSocket *server, AsyncWebSocketClient *client, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  String msg = "";
  if (info->final && info->index == 0 && info->len == len)
  {
    //the whole message is in a single frame and we got all of it's data
    if (info->opcode == WS_TEXT)
    {
      for (size_t i = 0; i < info->len; i++)
      {
        msg += (char)data[i];
      }
    }
    else
    {
      char buff[3];
      for (size_t i = 0; i < info->len; i++)
      {
        sprintf(buff, "%02x ", (uint8_t)data[i]);
        msg += buff;
      }
    }

    // Process Message
    StaticJsonDocument<JSON_CAPACITY> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err)
    {
      Serial.print(F("\ndeserializeJson() failed with code "));
      Serial.println(err.c_str());
    }
    else
    {
      // Proceed
      if (doc["type"] == "sensor-data")
      {
        Serial.println(msg);
        // doc["rssi"];
        // doc["data"]["wetness"];
        // doc["data"]["temperature"];
        // doc["data"]["humidity"];
        // doc["data"]["luminance"];
        // StaticJsonDocument<200> dat = doc["data"].as<JsonObject>();
        // Serial.printf("\n Sensor %.2f\n", dat["temperature"]);
        // Serial.printf("\n Sensor %.2f\n", dat["temperature"].as<float>());
        // Serial.printf("\n Sensor %.2f\n", doc["data"]["temperature"]);
        myStorage.push(doc);
        Serial.println("Ok sensor data received");
      }
      else if (doc["type"] == "sensor-register")
      {
        sensorIp = doc["ip"].as<char *>();
        sensorClientId = client->id();
        Serial.printf("\nRegistered new sensor device: %s (%s)\n", doc["deviceId"].as<char *>(), sensorIp);
      }
      else if (doc["type"] == "motor-register")
      {
        motorResponded = true;
        motorIp = doc["ip"].as<String>();
        motorClientId = client->id();
        Serial.printf("\nRegistered new motor device: %s (%s)\n", doc["deviceId"].as<char *>(), motorIp);
      }
      else if (doc["type"] == "motor-check")
      {
        motorResponded = true;
        motorStatus = doc["motorStatus"].as<char *>();
        Serial.println("Motor Node Checking In");
        Serial.print(motorStatus);
        Serial.print(doc["message"].as<char *>());
        // Serial.printf("%s : %s\n", motorStatus.c_str(), doc["message"].as<char *>());
      }
      else if (doc["type"] == "motor-respond")
      {
        motorResponded = true;
        motorStatus = doc["motorStatus"].as<char *>();
        Serial.printf("\nMotor Responded: %s \n", doc["deviceId"].as<char *>(), doc["message"]);
      }
      else
      {
        Serial.println("\nUnregistered WS Command");
      }
    }

    if (info->opcode == WS_TEXT)
      client->text("I got your text message");
  }
  else
  {
    client->text("Not accepting binary message!");
  }
}

// WS Event Handler
void WebSocketServer::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("\nWebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("\nWebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    Serial.printf("\nMessage Received from #%u: ", client->id());
    handleWebSocketMessage(arg, server, client, data, len);
    break;
  case WS_EVT_PONG:
    Serial.printf("\nGot Ping from #%u", client->id());
    client->text("Ping Received!");
    break;
  case WS_EVT_ERROR:
    Serial.println("WebSocket Event Error");
    break;
  }
}

void WebSocketServer::initWebSocket()
{
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}

String WebSocketServer::fetchInfo()
{
  StaticJsonDocument<200> doc;
  String out;

  JsonObject motor = doc.createNestedObject("motor");
  JsonObject sensor = doc.createNestedObject("sensor");
  if (motorResponded)
  {
    motor["status"] = true;
    motor["ip"] = motorIp;
    motor["wsId"] = motorClientId;
  }
  else
  {
    motor["status"] = false;
    motor["ip"] = "";
    motor["wsId"] = "";
  }
  bool sensorAvailable = false;
  if (myStorage.getStorage()[0] != NULL)
  {
    unsigned long lastTimeSensor;
    lastTimeSensor = myStorage.getStorage()[0]["timestamp"].as<unsigned long>();
    sensorAvailable = millis() - lastTimeSensor < 10000;
  }
  if (sensorAvailable)
  {
    sensor["status"] = true;
    sensor["ip"] = sensorIp;
    sensor["wsId"] = sensorClientId;
  }
  else
  {
    sensor["status"] = false;
    sensor["ip"] = "";
    sensor["wsId"] = "";
  }

  serializeJson(doc, out);

  return out;
}

void WebSocketServer::initWebRoute()
{
  server.on(
      "/check", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        Serial.println("\n[HTTP Received] Server Checked");
        request->send_P(200, "text/plain", "Ok");
      });
  server.on(
      "/info", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send(200, "application/json", fetchInfo()); });
  server.on(
      "/sensor/latest", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send(200, "application/json", myStorage.fetchAllSerializedJson()); });
  server.on(
      "/sensor/index", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        int paramsNr = request->params();
        int requestedIndex = 0;
        for (int i = 0; i < paramsNr; i++)
        {
          AsyncWebParameter *p = request->getParam(i);
          if (p->name() == "id")
            requestedIndex = (int)strtol(p->value().c_str(), (char **)NULL, 10);
        }
        request->send(200, "application/json", myStorage.fetchSerializedJson(requestedIndex));
      });

  server.on(
      "/motor/status", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/plain", ESWifi::getCoverStatus().c_str()); });

  // Server Motor Control
  server.on(
      "/motor/set/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/plain", ESWifi::setCoverStatus(motorStatus == "closed" ? "open" : "close") ? "Sent" : "Failed"); });
  server.on(
      "/motor/set/open", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/plain", ESWifi::setCoverStatus("open") ? "Sent" : "Failed"); });
  server.on(
      "/motor/set/close", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/plain", ESWifi::setCoverStatus("close") ? "Sent" : "Failed"); });
}

void WebSocketServer::wsLoop()
{
  ws.cleanupClients();
}