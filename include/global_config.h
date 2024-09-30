#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

typedef struct {
    uint8_t valveGPIO;
    bool valveState; 
} electroValvesStruct;
extern electroValvesStruct electroValves[4];
extern const uint8_t AMOUNT_OF_VALVES;
extern uint8_t hydraulicPump;

//global routine stuff
extern const char* CONFIG_PATH;
extern const char* DEFAULT_ROUTINE_CONFIG;

//wifi stuff
extern const char* ssid;
extern const char* password;
extern const bool STA_mode; // ? Capaz solo se tiene que usar en wifi_handler.h
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secundaryDNS;

//extern se utiliza para no redeclarar variables o funciones cuando se usan como variables
// globales o se usan en multiples archivos (sino cada vez que en un archivo se llame a
//este header se redeclaran los variables)

#endif //GLOBAL_CONFIG_H