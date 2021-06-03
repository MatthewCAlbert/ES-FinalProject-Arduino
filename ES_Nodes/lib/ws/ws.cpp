#include "ws.h"

const char *websockets_server_host = "10.10.1.1"; //Enter server adress
const uint16_t websockets_server_port = 80;       // Enter server port

using namespace websockets;

static bool replied;

void WS::connectToWs()
{
  if (client.connect(websockets_server_host, websockets_server_port, "/ws"))
  {
    Serial.println("WS Connected!");
  }
}

void WS::disconnect()
{
  client.close();
}

void WS::setReplied(bool status)
{
  replied = status;
}

bool WS::getReplied()
{
  return replied;
}

bool WS::getWsHealth()
{
  return client.available();
}

void WS::onMessageCallback(WebsocketsMessage message)
{
  setReplied(true);
  Serial.print("\nGot Message: ");
  Serial.println(message.data());
}

void WS::onEventsCallback(WebsocketsEvent event, String data)
{
  if (event == WebsocketsEvent::ConnectionOpened)
  {
    Serial.println("\nConnnection Opened");
  }
  else if (event == WebsocketsEvent::ConnectionClosed)
  {
    Serial.println("\nConnnection Closed");
  }
  else if (event == WebsocketsEvent::GotPing)
  {
    Serial.println("Got a Ping!");
  }
  else if (event == WebsocketsEvent::GotPong)
  {
    Serial.println("Got a Pong!");
  }
}

void WS::initWsClient()
{
  // run callback when messages are received
  client.onMessage(onMessageCallback);

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  connectToWs();
}

bool WS::poll()
{
  return client.poll();
}

bool WS::sendJson(String message)
{
  return client.send(message);
}