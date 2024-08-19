#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

typedef struct {
    uint8_t valveGPIO;
    bool valveState; 
} electroValvesStruct;
electroValvesStruct electroValves[4] = { {12,0}, {12,0}, {12,0}, {12,0} };
uint8_t hydraulicPump = 10;
bool rutineAuto = false, rutineManual = false, lastManualRutine = false, rutineON = false, timeFlag = true, pinUpdateFlag = true;
int8_t rutineIndex = 0;
unsigned long updatedTimes[sizeof(electroValves)/sizeof(electroValves[0])];
StaticJsonDocument<512> configRutina;
#define CONFIG_PATH "/config.json"
#define DEFAULT_RUTINE_CONFIG '{"days" : [false, false, false, false, false, false, false], "horarios" : [ [0,0], [0,0], [0,0], [0,0]],"enabled" : [false, false, false, false]}'

const char* ssid = "Flia Perez";
const char* password = "ponecualquiera";
//ip fija para no tener que ir adivinando
IPAddress local_IP(192,168,54,200);
IPAddress gateway(192,168,54,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secundaryDNS(8,8,4,4);
ESP8266WebServer server(80);

//function definitions
bool requestHandler(bool *error, int8_t *errorType);
void HTTP_handleRoot(void);
void rutineHandler();

void setup() {
    Serial.begin(115200);
    Serial.println("Buenas");
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    // Conectar a wifi 
    if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secundaryDNS)) Serial.println("La configuracion de IP fija fallo");
    WiFi.begin(ssid, password);
    Serial.print("Conectando...");
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.print("Conectado con éxito, mi IP es: ");
    Serial.println(WiFi.localIP());
    WiFi.setOutputPower(20.5);

    server.on ( "/", HTTP_handleRoot);
    //TODO: AGREGAR LA PARTE DE CONFIGURACION DE RUTINA
}

void loop() {
    server.handleClient();
    rutineHandler();
}

//------------------------------/   Funciones   /----------------------------------
void HTTP_handleRoot(void){
    int8_t errorType = 0;
    bool err = requestHandler(&server, &errorType);

    //Esta parte se encarga de hacer la lista de estado de retorno al cliente
    char statusAsCharJson[200];
    StaticJsonDocument<200> docdoc;
    for(uint_fast8_t i=0; i<sizeof(electroValves)/sizeof(electroValves[0]);i++) docdoc.add(electroValves[i].valveState);
    docdoc.add(rutineManual);
    docdoc.add(rutineAuto);
    serializeJson(docdoc, statusAsCharJson, 200);

    if(!err) server.send(200, "plain/text", statusAsCharJson);
    else server.send(500, "plain/text", errorType ? "Error con la conversion del JSON" : "Error con el disco interno del controlador de riego");
}

bool requestHandler(ESP8266WebServer *server, int8_t *errorType){
    for(uint_fast8_t i = 0; i < server->args(); i++){
        if(server->argName(i) == "state"){
            String clientInfo = server->arg(i);
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, clientInfo);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                *errorType =  0;
                return false;
            }
            for(uint_fast8_t pin = 0; pin < sizeof(electroValves)/sizeof(electroValves[0]); pin++) electroValves[pin].valveState = doc[pin];
            updatePins();
            rutineManual = doc[(sizeof(electroValves)/sizeof(electroValves[0]))+1];
        }
        if(server->argName(i) == "rutineState"){
            String clientInfo = server->arg(i);
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, clientInfo);
            rutineAuto = doc.as<bool>();
        }
        return true;
    }
}

void updatePins(){
    bool somethingIsOn = false;
    for(uint_fast8_t pin = 0; pin < sizeof(electroValves)/sizeof(electroValves[0]); pin++){
        digitalWrite(electroValves[pin].valveGPIO, electroValves[pin].valveState);
        if(electroValves[pin].valveState) somethingIsOn = true;
    }
    if(somethingIsOn) digitalWrite(hydraulicPump, true);
    else digitalWrite(hydraulicPump, false);
}

void rutineHandler(){
    for(uint_fast8_t i = 0; i < sizeof(electroValves)/ sizeof(electroValves[0]); i++) if(false && timeFlag) rutineON = true, timeFlag = false;    //Este if se encarga de habilitar la rutina segun los horarios
    if(rutineAuto || rutineManual){
        if(rutineON || (rutineManual != lastManualRutine)){             //Este if asigna las condiciones iniciales si recien se empieza el bucle de rutina
            rutineIndex = 0;
            rutineON = false;
            lastManualRutine = rutineManual;
            pinUpdateFlag = true;
            for(uint_fast8_t i = 0; i < sizeof(updatedTimes); i++){
                //TODO: aca va una funcion que suma los milisegundos actuales a el tiempo predefinido para esta instancia de rutina
            }
        }
        if(millis() < updatedTimes[rutineIndex] && pinUpdateFlag){      //Escribe los pines segun en que indice de rutina este, tambien se asegura de apagar la electro valvula anterior
            electroValves[rutineIndex - 1].valveGPIO = false;
            electroValves[rutineIndex].valveGPIO = true;
            updatePins();
            pinUpdateFlag = false;  //es para no spamear el digitalWrite a todo lo que da la CPU
        }else if(rutineIndex < sizeof(electroValves)/sizeof(electroValves[0])){ //aumenta el indice del bucle
            rutineIndex++;
            pinUpdateFlag = true;
        }else if(rutineIndex > sizeof(electroValves)/sizeof(electroValves[0])){
            electroValves[rutineIndex].valveGPIO = false;
            updatePins();
            rutineIndex = 0;
            timeFlag = true;
            if(rutineManual) rutineManual = false, lastManualRutine = false;
        }
    }
}