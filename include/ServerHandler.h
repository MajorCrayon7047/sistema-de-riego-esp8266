#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#include <ESP8266WebServer.h>
#include "Routine.h"

class ServerHandler {
public:
    ServerHandler(Routine* routine);
    void initServer();
    void handleClient();

private:
    ESP8266WebServer HTTP_server;
    Routine* routine;
    // Manejadores de las rutas
    void HTTP_handleRoot();
    void HTTP_handleConfig();
    bool requestHandler(int8_t* errorType);
};

#endif // SERVER_HANDLER_H