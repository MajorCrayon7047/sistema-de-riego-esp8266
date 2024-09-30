#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#include <ESP8266WebServer.h>

extern ESP8266WebServer HTTP_server;

void initServer(void);
bool requestHandler(ESP8266WebServer* HTTP_server, int8_t *errorType);
void HTTP_handleRoot(void);
void HTTP_handleConfig(void);

#endif // SERVER_HANDLER_H