#include <Arduino.h>
#include <ArduinoJson.h>
#include <SensorStorage.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>

class ESWifi
{
public:
  static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
  static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
  static void pingMotor();
  static String getCoverStatus();
  static bool setCoverStatus(String command);
};

class WebSocketServer
{
public:
  static void notifyClients();
  static void handleWebSocketMessage(void *arg, AsyncWebSocket *server, AsyncWebSocketClient *client, uint8_t *data, size_t len);
  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  static void initWebSocket();
  static void initWebRoute();
  static void wsLoop();
  static StaticJsonDocument<300> fetchInfo();
  static String fetchInfoSerialized();

private:
  static String authorizedSensorId[];
};

class MQTTClient
{
public:
  static void callback(char *topic, byte *payload, unsigned int length);
};

void initNtp();
String getTimestamp();
NTPClient esTimeClient();
String getMqttSendPacketData();