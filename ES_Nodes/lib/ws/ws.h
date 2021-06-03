#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

using namespace websockets;

static WebsocketsClient client;

class WS
{
public:
  static void connectToWs();
  static void disconnect();
  static bool getWsHealth();
  static bool getReplied();
  static void setReplied(bool status);
  static void onMessageCallback(WebsocketsMessage message);
  static void onEventsCallback(WebsocketsEvent event, String data);
  static void initWsClient();
  static bool sendJson(String message);
  static bool poll();
};