#include "global_config.h"

electroValvesStruct electroValves[4] = { {12,0}, {13,0}, {14,0}, {15,0} };
const uint8_t AMOUNT_OF_VALVES = sizeof(electroValves)/sizeof(electroValves[0]);
uint8_t hydraulicPump = 10;

//routine stuff
bool routineAuto = false, routineState = false;
const char* CONFIG_PATH = "/config.json";
const char* DEFAULT_ROUTINE_CONFIG = "{'duration':[0,0,0,0], 'days' : [false, false, false, false, false, false, false], 'horarios' : [ [99,99], [99,99], [99,99], [99,99] ]}";

//wifi stuff
const char* ssid = "Flia Perez";
const char* password = "ponecualquiera";
const bool STA_mode = true;   // ? Capaz solo se tiene que usar en wifi_handler.h
IPAddress local_IP(192,168,54,169);
IPAddress gateway(192,168,54,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secundaryDNS(8,8,8,8);