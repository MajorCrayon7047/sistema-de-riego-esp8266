#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

// ? pruebas
#include "global_config.h"
#include "FS_manager.h"

int8_t routineIndex = 0;
JsonDocument configRutina;

ESP8266WebServer server(80);

//Definicion de funciones
bool requestHandler(ESP8266WebServer* server, int8_t *errorType);
void HTTP_handleRoot();
void routineHandler();
bool routineOnEnable();
void routineOnDisable();
void autoRoutineHandler();
bool loadRoutineConfig(bool autoModeState = false);
bool loadRoutineMode();
bool updateFile(const char* dir, const String* data);
void updatePins(bool onlyCheck = false);
void wifiKeepAlive();

//Task Stufff
uint8_t currentPin = 0, numberOfPins = AMOUNT_OF_VALVES - 1;
Scheduler runner;
Task routineHandlerTask(0, numberOfPins+2, &routineHandler);

void setup() {
    Serial.begin(115200);
    Serial.println("Buenas");
    configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");
    delay(1000);
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
        while(true);
    }
    Serial.print("\n\n                      ............                      \n                ........................                \n             ..............................             \n          ............::::.:::::::............          \n        .........:=**%  :=-=-=+*- .....:........        \n      ..........=*:.+% =#*%*===+@=   .-:..........      \n     ...........+++=%::.  .:==#%#.     :...........     \n    .............:#-   .... .. .*@-    .:............   \n   ...............:  ..+%@%*: .: :%*     ::......:....  \n  ...............:.  :-@@@@@@= .   *#      ....:=.....  \n ..............:.   :..%@@@@@=  :   **       .::....... \n ............::     .   :==-..:. -   #:   --:.......... \n............:.      .:=+*##%%@% .@@+:-@@#=--............\n...........:.   .::#@@@@@@@@@@@+@@@@@@@@*-..............\n..........:. :::...:%@@@@@@@@@@@%%@*=---=:..............\n.........:-::.......:@@**@@@@@.   -.....................\n ........::.............#@%%%@##%@-.................... \n .................::=:.:#@%%%@%%%@-.................... \n  ............:---:*@@:.-%@@#:::...==-................  \n  ...........:-:::#@@%%@%%%%=    -#-::==-:............  \n   .......=++-:::#%@%%%%%%%@= .=%%-::::::==::........   \n     ...-+%+::::*==#%%%%@%+:+#@@%-::::::-+++++++=..     \n      :=++#::::#. %=+%#+--*%*==%-::::::-+++++**+*.      \n       :+*::::*.  .%%=+++:   :#-:::::::++++*#+++-       \n         .:::*:  =*+:+%.    -#::::::::***+#*+=:         \n            -= =-.+ .--@-  =*::::::::*@@%@*-.           \n              . +%=.:@+.=-++::::::::-@@#=:      |Powered by: MajorCrayon7047            \n                   : -*#+**::::::.....          |Proyect Name: Controlador de Riego\n");
    runner.init();
    runner.addTask(routineHandlerTask);
    routineHandlerTask.setOnEnable(routineOnEnable);
    routineHandlerTask.setOnDisable(routineOnDisable);
    // Conectar a wifi 
    //if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secundaryDNS)) Serial.println("La configuracion de IP fija fallo");
    WiFi.begin(ssid, password);
    Serial.print("Conectando...");
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.print("Conectado con éxito, mi IP es: ");
    Serial.println(WiFi.localIP());
    WiFi.setOutputPower(20.5);
    /*
    WiFi.mode( WIFI_AP );//for AP mode
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Punto de acceso iniciado. IP: ");
    Serial.println(IP);
    */
    bool err = !(loadRoutineConfig() || loadRoutineConfig(true));
    if(err){
        Serial.println("Error cargando configuracion");
    //    while(true);
    }else{
        Serial.println("Se cargo la configuracion de rutina :)");
    }
    server.on("/", HTTP_handleRoot);
    server.on("/config", [](){
        int8_t errorType = 0;
        bool err = !requestHandler(&server, &errorType);
        char configAsCharJson[600];
        serializeJson(configRutina, configAsCharJson, sizeof(configAsCharJson));
        Serial.println(configAsCharJson);
        if(!err) server.send(200, "plain/text", configAsCharJson);
        else server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
    });
    server.onNotFound(HTTP_handleRoot);
    server.begin();
}

void loop(){
    wifiKeepAlive();
    server.handleClient();
    runner.execute();
    if(routineAuto)autoRoutineHandler();
    //wifiKeepAlive();
}

//------------------------------/   Funciones   /----------------------------------
void HTTP_handleRoot(){
    Serial.println("AAAAAAAA");
    int8_t errorType = 0;
    bool err = !requestHandler(&server, &errorType);

    //Esta parte se encarga de hacer la lista de estado de retorno al cliente
    char statusAsCharJson[200];
    JsonDocument docdoc;
    for(uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++) docdoc.add(electroValves[i].valveState);
    docdoc.add(routineState);
    docdoc.add(routineAuto);
    serializeJson(docdoc, statusAsCharJson, sizeof(statusAsCharJson));
    Serial.println(statusAsCharJson);

    if(!err) server.send(200, "plain/text", statusAsCharJson);
    else server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

bool requestHandler(ESP8266WebServer* server, int8_t *errorType){
    DeserializationError error;
    bool errorFile = false;
    for(int i = 0; i < server->args(); i++){
        if(server->argName(i) == "state"){
            String clientInfo = server->arg(i);
            JsonDocument doc;
            error = deserializeJson(doc, clientInfo);
            if (error) continue;    //salta directamente de esta iteracion a la siguiente y despues se ejecutara el if de error para el return false
            for(uint_fast8_t pin = 0; pin < AMOUNT_OF_VALVES; pin++) electroValves[pin].valveState = doc[pin];
            updatePins();
            routineState = doc[AMOUNT_OF_VALVES].as<bool>();
            if(doc[AMOUNT_OF_VALVES].as<bool>()) routineHandlerTask.enableIfNot();
            else routineHandlerTask.disable();
        }
        if(server->argName(i) == "routineState"){
            errorFile = (!updateFile("/routine.txt", &server->arg(i))) || errorFile;     //the OR is to ensure that the value is not always changing
            routineAuto = server->arg(i) == "true" || server->arg(i) == "1";
        }
        if(server->argName(i) == "data"){
            errorFile = (!updateFile(CONFIG_PATH, &server->arg(i))) || errorFile;
            loadRoutineConfig();
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

bool updateFile(const char* dir, const String* data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(*data);
    Serial.println("Se imprimieron los siguientes datos: " + *data);
    return true;
}
bool updateFile(const char* dir, const char* data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(data);
    //Serial.println("Se imprimieron los siguientes datos: " + *data);
    return true;
}

void updatePins(bool onlyCheck){
    bool somethingIsOn = false;
    for(uint_fast8_t pin = 0; pin < AMOUNT_OF_VALVES; pin++){
        if(!onlyCheck) digitalWrite(electroValves[pin].valveGPIO, electroValves[pin].valveState);
        if(electroValves[pin].valveState) somethingIsOn = true;
    }
    if(somethingIsOn) digitalWrite(hydraulicPump, true);
    else digitalWrite(hydraulicPump, false);
}

void autoRoutineHandler(){
    int hora = 98, minuto = 98, dayOfTheWeek = 0;
    for(uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++){
        if((configRutina["horarios"][i][0].as<int>() == hora)  && (configRutina["horarios"][i][1].as<int>() == minuto) && configRutina["days"][dayOfTheWeek].as<bool>() && !routineState){
            routineState = true;    //Este if se encarga de habilitar la rutina segun los horarios
            routineHandlerTask.enableIfNot();
        }
    }
    //if(routineState && !routineHandlerTask.isEnabled()) routineHandlerTask.enableIfNot();
}
void routineHandler(){
    Serial.printf("\nCurrent Pin: %d", currentPin);
    Serial.printf("\nEjecutando el intervalo: %ld", routineHandlerTask.getIterations());
    if(currentPin > 0) electroValves[currentPin-1].valveState = false;
    if(currentPin > 0) Serial.printf("\nApagando el pin %d", electroValves[currentPin-1].valveGPIO);
    if(routineHandlerTask.isLastIteration()){
        electroValves[numberOfPins].valveState = false;
        Serial.printf("\nApagando el pin %d", electroValves[numberOfPins].valveGPIO);
        routineHandlerTask.disable();
        Serial.println("\nLLegamos a la ultima iteracion, se cumplio la rutina");
        routineState = false;
        return;
    }
    electroValves[currentPin].valveState = true;
    Serial.printf("\nEncendiendo el pin %d", electroValves[currentPin].valveGPIO);
    routineHandlerTask.setInterval(configRutina["duration"][currentPin].as<unsigned long>());
    updatePins();
    currentPin++;
}

bool routineOnEnable() {
    Serial.println("Seteando variables iniciales");
    currentPin = 0;
    return  true;
}

void routineOnDisable(){
    Serial.println("\nDeshabilitando la rutina");
    //digitalWrite(pins[currentPin], false);
    currentPin = 0;
}

bool loadRoutineConfig(bool autoState){
    const char* falso = "false";
    if(!LittleFS.exists("routine.txt")) updateFile("routine.txt", falso);
    if(!LittleFS.exists(CONFIG_PATH)) updateFile(CONFIG_PATH, DEFAULT_ROUTINE_CONFIG);
    File file = LittleFS.open(!autoState ? CONFIG_PATH : "routine.txt" , "r");
    if(!file){
        Serial.println("Error cargando la configuracion default, debido al sistema de archivos");
        return false;
    }
    if(!autoState){
        DeserializationError error =  deserializeJson(configRutina, file.readString());
        if(error){
            Serial.println("Error cargando la configuracion default");
            Serial.println(error.c_str());
            return false;
        }
    }
    else{
        String routineValue = file.readString();
        routineAuto = routineValue == "true" || routineValue == "1";
    }
    file.close();
    return true;
}

void wifiKeepAlive(){
    if ( WiFi.status() != WL_CONNECTED ){
    int tries = 0;
    WiFi.begin( ssid, password );
    while( WiFi.status() != WL_CONNECTED ) {
        tries++;
        Serial.println( "|" );
        if ( tries == 500 ) ESP.reset();
        delay( 50 );
    }
    WiFi.setOutputPower(20.5);
    Serial.print( "Connected " );
    Serial.println( WiFi.localIP() );
    }
}