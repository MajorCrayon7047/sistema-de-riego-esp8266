#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server;

bool requestHandler(ESP8266WebServer* server, int8_t *errorType);
void HTTP_handleRoot(void);
void HTTP_handleConfig(void);