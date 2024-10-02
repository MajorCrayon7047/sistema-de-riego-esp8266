#include "Routine.h"
#include "global_config.h"
#include "utils.h"

//TODO: Implementar un sistema de LOG

// Constructor de la clase Routine
Routine::Routine() {
    // Inicializa la tarea de la rutina con un handler y un intervalo
    this->routineTask = new Task([this]() { this->routineHandler(); }, 0, AMOUNT_OF_VALVES + 2);
    this->currentPin = 0;
    this->numberOfPins = AMOUNT_OF_VALVES - 1;
    this->routineAuto = false;
    this->routineState = false;

    // Inicializa el manejador de tiempo con un intervalo de 60000 ms (1 minuto)
    this->time = new MultiTimeHandler(60000);
}

// Método para inicializar la rutina
void Routine::begin(){
    time->begin();
    
    // Configura los handlers para cuando la tarea se habilita y deshabilita
    routineTask->setOnEnable([this]() { this->routineOnEnable(); });
    routineTask->setOnDisable([this]() { this->routineOnDisable(); });

    // Carga la configuración y verifica si hubo un error
    bool err = !(loadConfig() || loadConfig(true));
    if(err){
        Serial.println("Error cargando configuracion");
        //while(true); // Bucle infinito en caso de error
    }
    else Serial.println("Se cargo la configuracion de rutina :)");
}

// Método para habilitar la rutina
void Routine::enable(){
    routineTask->setInterval(0);
    routineTask->enable();
}

// Método para deshabilitar la rutina
void Routine::disable(){
    routineTask->disable();
}

// Método privado para cargar la configuración
bool Routine::loadConfig(bool autoModeState) {
    const char* falso = "false";
    // Verifica si los archivos de configuración existen, si no, los crea con valores por defecto
    if(!doFileExists(ROUTINE_ENABLE_PATH)) updateFile(ROUTINE_ENABLE_PATH, falso);
    if(!doFileExists(CONFIG_PATH)) updateFile(CONFIG_PATH, DEFAULT_ROUTINE_CONFIG);
    //TODO: arreglar el que por alguna razon DEFAULT_ROUTINE_CONFIG se guarda como "{" en vez de su valor default correcto
    //TODO: agregar routine.txt a global_config.h
    //Serial.println(DEFAULT_ROUTINE_CONFIG);

    // Lee el contenido del archivo de configuración
    String fileContent = readFile(!autoModeState ? CONFIG_PATH : "routine.txt");
    if(fileContent == "") return false;
    
    // Deserializa el JSON de configuración
    if(!autoModeState){
        DeserializationError error =  deserializeJson(configRutina, fileContent);
        if(error){
            Serial.printf("Error cargando la configuracion default, el error es: %s\n", error.c_str());
            Serial.printf("El contenido del archivo %s es: ", !autoModeState ? CONFIG_PATH : ROUTINE_ENABLE_PATH);
            Serial.print(fileContent + "\n");
            return false;
        }
    }
    else{
        routineAuto = fileContent == "true" || fileContent == "1";
    }
    return true;
}

// Handler de la rutina, se ejecuta en cada intervalo
void Routine::routineHandler() {
    Serial.printf("\nCurrent Pin: %d", currentPin);
    Serial.printf("\nEjecutando el intervalo: %u", routineTask->getCurrentIteration());
    
    // Apaga el pin anterior si no es el primero
    if(currentPin > 0) electroValves[currentPin-1].valveState = false;
    if(currentPin > 0) Serial.printf("\nApagando el pin %d", electroValves[currentPin-1].valveGPIO);
    
    // Si es la última iteración, apaga el último pin y deshabilita la tarea
    if(routineTask->isLastIteration()){
        electroValves[numberOfPins].valveState = false;
        Serial.printf("\nApagando el pin %d", electroValves[numberOfPins].valveGPIO);
        routineTask->disable();
        Serial.println("\nLLegamos a la ultima iteracion, se cumplio la rutina");
        routineState = false;
        return;
    }
    
    // Enciende el pin actual y actualiza el intervalo
    electroValves[currentPin].valveState = true;
    Serial.printf("\nEncendiendo el pin %d", electroValves[currentPin].valveGPIO);
    routineTask->setInterval(configRutina["duration"][currentPin].as<unsigned long>());
    updatePins(false);
    currentPin++;
}

// Handler que se ejecuta cuando la tarea se habilita
void Routine::routineOnEnable() {
    Serial.println("Seteando variables iniciales");
    currentPin = 0;
}

// Handler que se ejecuta cuando la tarea se deshabilita
void Routine::routineOnDisable(){
    Serial.println("\nDeshabilitando la rutina");
    //digitalWrite(pins[currentPin], false); // Para apagar el pin actual
    updatePins(false);
    currentPin = 0;
}

// Handler principal que se llama periódicamente
void Routine::handler() {
    routineTask->handler();
    time->update();             // ! quitar mas tarde (prueba)

    // Si la rutina está en modo automático, actualiza el tiempo y verifica los horarios
    if(routineAuto){
        time->update();                 // Se actualiza el tiempo cada minuto

        // Verifica si es hora de habilitar la rutina según la configuración
        for(uint_fast8_t i = 0; i < AMOUNT_OF_VALVES; i++){
            if((configRutina["horarios"][i][0].as<int>() == time->hour)  && 
               (configRutina["horarios"][i][1].as<int>() == time->minute) && 
               ( (time->dayOfTheWeek != 99) ? configRutina["days"][time->dayOfTheWeek].as<bool>() : false ) && 
               !routineState){
                routineState = true;    // Este if se encarga de habilitar la rutina según los horarios
                routineTask->enableIfNot();
            }
        }
        //if(routineState && !routineTask.isEnabled()) routineTask.enableIfNot(); // Comentado, posiblemente redundante
    }
}