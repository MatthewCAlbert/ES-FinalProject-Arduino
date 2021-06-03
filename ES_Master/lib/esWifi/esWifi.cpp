#include "esWifi.h"

static String sensorIp;
String motorIp;
SensorStorage myStorage;

int connectedDevice = 0;
void ESWifi::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  connectedDevice++;
}
void ESWifi::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  connectedDevice--;
}
void ESWifi::setCoverStatus(bool status)
{
  HTTPClient http;
  String serverPath = "http://" + motorIp + "/status/set/" + (status ? "1" : "0");
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
  }
  // Free resources
  http.end();
}

// Websockets
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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
        Serial.println("\nOk sensor data received");
      }
      else if (doc["type"] == "sensor-register")
      {
        sensorIp = doc["ip"].as<char *>();
        Serial.printf("\nRegistered new sensor device: %s (%s)\n", doc["deviceId"], sensorIp);
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
    Serial.printf("\nMessage Received from #%u", client->id());
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

void WebSocketServer::initWebRoute()
{
  server.on(
      "/check", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/html", "Ok"); });
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
}

void WebSocketServer::wsLoop()
{
  ws.cleanupClients();
}