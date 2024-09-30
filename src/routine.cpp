#include "routine.h"

#include "global_config.h"
#include "utils.h"
#include <TaskScheduler.h>

JsonDocument configRutina;
uint8_t currentPin = 0, numberOfPins = AMOUNT_OF_VALVES - 1;
//Scheduler taskHandler;
//Task routineHandlerTask(0, numberOfPins + 2, &routineHandler);
bool routineAuto = false, routineState = false;

void initRoutineConfig(){
    taskHandler.init();
    taskHandler.addTask(routineHandlerTask);
    routineHandlerTask.setOnEnable(routineOnEnable);
    routineHandlerTask.setOnDisable(routineOnDisable);

    bool err = !(loadRoutineConfig() || loadRoutineConfig(true));
    if(err){
        Serial.println("Error cargando configuracion");
    //    while(true);
    }
    else Serial.println("Se cargo la configuracion de rutina :)");
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
    updatePins(false);
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
    if(!doFileExists("routine.txt")) updateFile("routine.txt", falso);
    if(!doFileExists(CONFIG_PATH)) updateFile(CONFIG_PATH, DEFAULT_ROUTINE_CONFIG);
    String fileContent = readFile(!autoState ? CONFIG_PATH : "routine.txt");
    if(fileContent == "") return false;
    if(!autoState){
        DeserializationError error =  deserializeJson(configRutina, fileContent);
        if(error){
            Serial.println("Error cargando la configuracion default");
            Serial.println(error.c_str());
            return false;
        }
    }
    else{
        routineAuto = fileContent == "true" || fileContent == "1";
    }
    return true;
}

void enableRoutine(){
    routineHandlerTask.enable();
}
void disableRoutine(){
    routineHandlerTask.disable();
}

void executeTaskHandler(){
    taskHandler.execute();
}