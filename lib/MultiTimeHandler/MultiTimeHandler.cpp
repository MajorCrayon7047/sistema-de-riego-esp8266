#include "MultiTimeHandler.h"
#include <SPI.h>

// Constructor de la clase MultiTimeHandler
MultiTimeHandler::MultiTimeHandler(unsigned long updateInterval_MS) 
    :   ntpUDP(),                                           // Inicializa ntpUDP primero
        timeClient(ntpUDP, "pool.ntp.org", (3600 * (-3))) { // Inicializa el cliente NTP con la zona horaria UTC-3
    // Crea una tarea para actualizar el tiempo a intervalos regulares
    this->updateTimeTask = new Task([this]() { this->getTime(); }, updateInterval_MS, 0);
    this->hour = 99;                    // Inicializa la hora a un valor no válido
    this->minute = 99;                  // Inicializa el minuto a un valor no válido
    this->rtcIsNotWorking = false;      // Inicializa el estado del RTC como funcional
    this->ntptimeIsNotWorking = false;  // Inicializa el estado del NTP como funcional
}

// Método para inicializar el RTC y el cliente NTP
void MultiTimeHandler::begin(){
    uint8_t tries = 0;
    while (! rtc.begin()) {         // Intenta inicializar el RTC
        Serial.println("Couldn't find RTC, retrying...");
        tries++;
        if(tries > 5){              // Si no se puede inicializar después de 5 intentos, marca el RTC como no funcional
            rtcIsNotWorking = true;
            break;
        }
    }
    
    timeClient.begin();         // Inicializa el cliente NTP
    updateTimeTask->enable();   // Habilita la tarea de actualización de tiempo
}

// Método para configurar el tiempo del RTC
void MultiTimeHandler::setupTime(const char* date, const char* time){
    if(!rtcIsNotWorking){
        // Ajusta el RTC a la fecha y hora proporcionadas
        rtc.adjust(DateTime(date, time));
    }
}

// Método para establecer el intervalo de actualización
void MultiTimeHandler::setUpdateInterval(unsigned long updateInterval_MS){
    updateTimeTask->setInterval(updateInterval_MS);
}

// Método para obtener el tiempo actual
void MultiTimeHandler::getTime(){
    if(timeClient.forceUpdate() && rtcIsNotWorking){        // Si el NTP funciona y el RTC no, obtiene el tiempo del NTP
        hour = timeClient.getHours();
        minute = timeClient.getMinutes();
        dayOfTheWeek = timeClient.getDay();             // 0 es Domingo
    }
    else if(rtcIsNotWorking){                           // Si ninguno funciona, establece valores no válidos
        hour = 99;
        minute = 99;
        dayOfTheWeek = 99;
    }
    else{                                               // Si el RTC funciona, obtiene el tiempo del RTC
        now = rtc.now();
        hour = now.hour();
        minute = now.minute();
        dayOfTheWeek = now.dayOfTheWeek();
    }
    Serial.printf(" %s Time: %02d:%02d, Day: %d\n", rtcIsNotWorking? "NTP" : "RTC", hour, minute, dayOfTheWeek);    // ! debug
}

// Método para actualizar la tarea de tiempo
void MultiTimeHandler::update(){
    updateTimeTask->handler();
}

// Método para forzar una actualización del tiempo
void MultiTimeHandler::forceUpdate(){
    getTime();
}

// Métodos para obtener la hora, minuto y día de la semana
uint8_t MultiTimeHandler::getHour(){
    return hour;
}

uint8_t MultiTimeHandler::getMinute(){
    return minute;
}

uint8_t MultiTimeHandler::getDayOfTheWeek(){
    return dayOfTheWeek;
}