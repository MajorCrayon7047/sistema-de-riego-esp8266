#include "Routine.h"

#include "global_config.h"
#include "utils.h"

// now public Stuff
Routine::Routine() {
    this->routineTask = new Task([this]() { this->routineHandler(); }, 0, AMOUNT_OF_VALVES + 2);
    this->currentPin = 0;
    this->numberOfPins = AMOUNT_OF_VALVES - 1;
    this->routineAuto = false;
    this->routineState = false;

    this->time = new MultiTimeHandler(60000);
}

void Routine::begin(){
    time->begin();
    
    routineTask->setOnEnable([this]() { this->routineOnEnable(); });
    routineTask->setOnDisable([this]() { this->routineOnDisable(); });

    bool err = !(loadConfig() || loadConfig(true));
    if(err){
        Serial.println("Error cargando configuracion");
    //    while(true);
    }
    else Serial.println("Se cargo la configuracion de rutina :)");
}

void Routine::enable(){
    routineTask->setInterval(0);
    routineTask->enable();
}

void Routine::disable(){
    routineTask->disable();
}

//private stuff
bool Routine::loadConfig(bool autoModeState) {
    const char* falso = "false";
    if(!doFileExists("routine.txt")) updateFile("routine.txt", falso);
    if(!doFileExists(CONFIG_PATH)) updateFile(CONFIG_PATH, DEFAULT_ROUTINE_CONFIG);
    String fileContent = readFile(!autoModeState ? CONFIG_PATH : "routine.txt");
    if(fileContent == "") return false;
    if(!autoModeState){
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

void Routine::routineHandler() {
    Serial.printf("\nCurrent Pin: %d", currentPin);
    Serial.printf("\nEjecutando el intervalo: %u", routineTask->getCurrentIteration());
    if(currentPin > 0) electroValves[currentPin-1].valveState = false;
    if(currentPin > 0) Serial.printf("\nApagando el pin %d", electroValves[currentPin-1].valveGPIO);
    if(routineTask->isLastIteration()){
        electroValves[numberOfPins].valveState = false;
        Serial.printf("\nApagando el pin %d", electroValves[numberOfPins].valveGPIO);
        routineTask->disable();
        Serial.println("\nLLegamos a la ultima iteracion, se cumplio la rutina");
        routineState = false;
        return;
    }
    electroValves[currentPin].valveState = true;
    Serial.printf("\nEncendiendo el pin %d", electroValves[currentPin].valveGPIO);
    routineTask->setInterval(configRutina["duration"][currentPin].as<unsigned long>());
    updatePins(false);
    currentPin++;
}

void Routine::routineOnEnable() {
    Serial.println("Seteando variables iniciales");
    currentPin = 0;
}

void Routine::routineOnDisable(){
    Serial.println("\nDeshabilitando la rutina");
    //digitalWrite(pins[currentPin], false);
    updatePins(false);
    currentPin = 0;
}


void Routine::handler() {
    routineTask->handler();
    if(routineAuto){
        time->update(); //se actualiza el tiempo cada minuto

        for(uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++){
            if((configRutina["horarios"][i][0].as<int>() == time->hour)  && (configRutina["horarios"][i][1].as<int>() == time->minute) && ( (time->dayOfTheWeek != 99) ? configRutina["days"][time->dayOfTheWeek].as<bool>() : false ) && !routineState){
                routineState = true;    //Este if se encarga de habilitar la rutina segun los horarios
                routineTask->enableIfNot();
            }
        }
        //if(routineState && !routineTask.isEnabled()) routineTask.enableIfNot();
    }
}