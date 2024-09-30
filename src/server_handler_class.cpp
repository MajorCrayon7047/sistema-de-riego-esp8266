#include "server_handler_class.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

#include "global_config.h"
#include "utils.h"
#include "routine_class.h"

ServerHandler::ServerHandler(Routine* routine) {
    ESP8266WebServer HTTP_server(80);
    this->routine = routine;
}

void ServerHandler::initServer() {
    HTTP_server.on("/", [this]() { HTTP_handleRoot(); });
    HTTP_server.on("/config", [this]() { HTTP_handleConfig(); });
    HTTP_server.onNotFound([this]() { HTTP_handleRoot(); });
    HTTP_server.begin();
}

void ServerHandler::HTTP_handleRoot() {
    int8_t errorType = 0;
    bool err = !requestHandler(&errorType);

    // Crear la lista de estado de retorno al cliente
    char statusAsCharJson[200];
    JsonDocument docdoc;
    for (uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++) {
        docdoc.add(electroValves[i].valveState);
    }
    docdoc.add(routine->routineState);
    docdoc.add(routine->routineAuto);
    serializeJson(docdoc, statusAsCharJson, sizeof(statusAsCharJson));
    Serial.println(statusAsCharJson);

    if(!err) HTTP_server.send(200, "plain/text", statusAsCharJson);
    else HTTP_server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

void ServerHandler::HTTP_handleConfig() {
    int8_t errorType = 0;
    bool err = !requestHandler(&errorType);
    char configAsCharJson[600];
    serializeJson(routine->configRutina, configAsCharJson, sizeof(configAsCharJson));
    Serial.println(configAsCharJson);

    if(!err) HTTP_server.send(200, "plain/text", configAsCharJson);
    else HTTP_server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

bool ServerHandler::requestHandler(int8_t* errorType) {
    DeserializationError error;
    bool errorFile = false;

    for (int i = 0; i < HTTP_server.args(); i++) {
        if (HTTP_server.argName(i) == "state") {
            String clientInfo = HTTP_server.arg(i);
            JsonDocument doc;
            error = deserializeJson(doc, clientInfo);
            if (error) continue; // Salta a la siguiente iteración
            for (uint_fast8_t pin = 0; pin < AMOUNT_OF_VALVES; pin++) {
                electroValves[pin].valveState = doc[pin];
            }
            updatePins(false);
            routine->routineState = doc[AMOUNT_OF_VALVES].as<bool>();
            routine->routineState ? routine->enableRoutine() : routine->disableRoutine();
        }

        else if (HTTP_server.argName(i) == "routineState") {
            errorFile = (!updateFile("/routine.txt", &HTTP_server.arg(i))) || errorFile;
            routine->routineAuto = HTTP_server.arg(i) == "true" || HTTP_server.arg(i) == "1";
        }

        else if (HTTP_server.argName(i) == "data") {
            errorFile = (!updateFile(CONFIG_PATH, &HTTP_server.arg(i))) || errorFile;
            routine->loadRoutineConfig(false);
        }
    }

    if (error && errorFile) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        *errorType = 0;
        return false;
    } else if (errorFile) {
        Serial.print(F("Error abriendo archivo"));
        *errorType = 1;
        return false;
    }

    return true;
}

void ServerHandler::handleClient() {
    HTTP_server.handleClient();
}