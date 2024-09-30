#include "server_handler.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

#include "global_config.h"
#include "utils.h"
#include "routine.h"

ESP8266WebServer HTTP_server(80);

void initServer(void){
    HTTP_server.on("/", HTTP_handleRoot);
    HTTP_server.on("/config", HTTP_handleConfig);
    HTTP_server.onNotFound(HTTP_handleRoot);
    HTTP_server.begin();
}

void HTTP_handleRoot(){
    int8_t errorType = 0;
    bool err = !requestHandler(&HTTP_server, &errorType);

    //Esta parte se encarga de hacer la lista de estado de retorno al cliente
    char statusAsCharJson[200];
    JsonDocument docdoc;
    for(uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++) docdoc.add(electroValves[i].valveState);
    docdoc.add(routineState);
    docdoc.add(routineAuto);
    serializeJson(docdoc, statusAsCharJson, sizeof(statusAsCharJson));
    Serial.println(statusAsCharJson);

    if(!err) HTTP_server.send(200, "plain/text", statusAsCharJson);
    else HTTP_server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

void HTTP_handleConfig(){
    int8_t errorType = 0;
    bool err = !requestHandler(&HTTP_server, &errorType);
    char configAsCharJson[600];
    serializeJson(configRutina, configAsCharJson, sizeof(configAsCharJson));
    Serial.println(configAsCharJson);
    if(!err) HTTP_server.send(200, "plain/text", configAsCharJson);
    else HTTP_server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

bool requestHandler(ESP8266WebServer* HTTP_server, int8_t *errorType){
    DeserializationError error;
    bool errorFile = false;
    for(int i = 0; i < HTTP_server->args(); i++){
        if(HTTP_server->argName(i) == "state"){
            String clientInfo = HTTP_server->arg(i);
            JsonDocument doc;
            error = deserializeJson(doc, clientInfo);
            if (error) continue;    //salta directamente de esta iteracion a la siguiente y despues se ejecutara el if de error para el return false
            for(uint_fast8_t pin = 0; pin < AMOUNT_OF_VALVES; pin++) electroValves[pin].valveState = doc[pin];
            updatePins();
            routineState = doc[AMOUNT_OF_VALVES].as<bool>();
            if(doc[AMOUNT_OF_VALVES].as<bool>()) enableRoutine();
            else disableRoutine();
        }
        if(HTTP_server->argName(i) == "routineState"){
            errorFile = (!updateFile("/routine.txt", &HTTP_server->arg(i))) || errorFile;     //the OR is to ensure that the value is not always changing
            routineAuto = HTTP_server->arg(i) == "true" || HTTP_server->arg(i) == "1";
        }
        if(HTTP_server->argName(i) == "data"){
            errorFile = (!updateFile(CONFIG_PATH, &HTTP_server->arg(i))) || errorFile;
            loadRoutineConfig(false);
        }
    }
    if (error && errorFile){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        *errorType =  0;
        return false;
    }else if (errorFile){
        Serial.print(F("Error abriendo archivo"));
        *errorType = 1;
        return false;
    }
    return true;
}