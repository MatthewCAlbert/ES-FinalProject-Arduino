#include "esWifi.h"

static HTTPClient http;
static WiFiClient client;

static String sensorIp;
String motorIp = "";
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
String ESWifi::getCoverStatus()
{
  Serial.println(motorIp);
  if (motorIp == "")
  {
    Serial.println("Motor node not connected");
    return "error";
  }
  if (!Ping.ping(motorIp.c_str(), 3))
  {
    return "server not responding";
  }
  String serverPath = "http://" + motorIp + "/status";
  bool httpInitResult = http.begin(client, serverPath.c_str());
  http.addHeader("Keep-Alive", "timeout=2, max=1");

  Serial.println("[INFO] Before sending GET");

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  Serial.println("[INFO] Before fetch payload");

  String payload;
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return "error";
  }
  // Free resources
  http.end();

  return payload;
}
bool ESWifi::setCoverStatus(String command)
{
  if (motorIp == "")
  {
    Serial.println("Motor node not connected");
    return false;
  }
  String serverPath = "http://" + motorIp + "/status/set/" + (command == "open" ? "open" : "close");
  bool httpInitResult = http.begin(client, serverPath.c_str());
  http.addHeader("Keep-Alive", "timeout=3, max=5");
  if (httpInitResult == false)
  {
    Serial.println("http.begin() failed!"); //debug
    return false;
  }
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
  if (httpResponseCode > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
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
      {
        Serial.println("\n[HTTP Received] Server Checked");
        request->send_P(200, "text/plain", "Ok");
      });
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
      "/motor/updateip", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        Serial.println("\n[HTTP Received] Motor IP Update");
        String motorDeviceId;

        if (request->hasParam("deviceId", true))
          motorDeviceId = request->getParam("deviceId", true)->value();

        if (request->hasParam("ip", true))
        {
          motorIp = request->getParam("ip", true)->value();
          Serial.printf("\n[Motor Node] Connected with IP:%s\n", motorIp.c_str());
          request->send(200, "text/plain", "Motor IP Received");
        }
        else
        {
          Serial.println("[HTTP Received] IP not included");
          request->send(400, "text/plain", "Motor IP Not Received");
        }
      });
  server.on(
      "/motor/status", HTTP_GET, [](AsyncWebServerRequest *request)
      { request->send_P(200, "text/plain", ESWifi::getCoverStatus().c_str()); });

  // Server Motor Control
  server.on(
      "/motor/set/open", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        request->send_P(200, "text/plain", "Ok");
        ESWifi::setCoverStatus("open");
      });
  server.on(
      "/motor/set/close", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        request->send_P(200, "text/plain", "Ok");
        ESWifi::setCoverStatus("close");
      });
}

void WebSocketServer::wsLoop()
{
  ws.cleanupClients();
}