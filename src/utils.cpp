#include <LittleFS.h>
#include <Arduino.h>
#include "utils.h"
#include "global_config.h"

void initFS(void){
    Serial.println("Iniciando LittleFS");
    int8_t tries = 0;
    while(!LittleFS.begin()){
        Serial.printf("An Error has occurred while mounting LittleFS, retrying in 2 seconds, try number %d\n", tries);
        delay(1000);
        tries++;
        if(tries == 10) ESP.restart();
    }
}

bool doFileExists(const char* dir){
    return LittleFS.exists(dir);
}

bool doFileExists(const String& dir){
    return LittleFS.exists(dir);
}

bool updateFile(const char* dir, const String& data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(data);
    file.close();
    Serial.println("Se guardaron los siguientes datos: " + data);
    return true;
}

bool updateFile(const char* dir, const char* data){
    File file = LittleFS.open(dir, "w");
    if(!file) return false;
    file.print(*data);
    file.close();
    Serial.println("Se guardaron los siguientes datos: " + *data);
    return true;
}

String readFile(const char* dir){
    File file = LittleFS.open(dir, "r");
    if(!file) {
        Serial.println("Error abriendo el archivo, en el directorio: " + String(dir));
        return "";
    }
    String fileContent = file.readString();
    file.close();
    return fileContent;
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