#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

// Set web server port number to 80
WebServer server(80);

void handle_PostSensor()
{
  server.send(200, "text/plain", "Data received");
}
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

void serveWeb()
{
  server.on("/sensor", handle_PostSensor);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}