#include "MultiTimeHandler.h"
#include <SPI.h>

MultiTimeHandler::MultiTimeHandler(unsigned long updateInterval_MS) 
    : timeClient(ntpUDP, "pool.ntp.org", 3600 * (-3)) {
    this->updateTimeTask = new Task([this]() { this->getTime(); }, updateInterval_MS, 0);
    this->hour = 99;
    this->minute = 99;
    this->rtcIsNotWorking = false;
    this->ntptimeIsNotWorking = false;
}

void MultiTimeHandler::begin(){
    uint8_t tries = 0;
    while (! rtc.begin()) {
        Serial.println("Couldn't find RTC, retrying...");
        tries++;
        if(tries > 5){
            rtcIsNotWorking = true;
            break;
        }
    }
    
    timeClient.begin();
    updateTimeTask->enable();
}

void MultiTimeHandler::setupTime(const char* date, const char* time){
    if(!rtcIsNotWorking){
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(date, time));
    }
}

void MultiTimeHandler::setUpdateInterval(unsigned long updateInterval_MS){
    updateTimeTask->setInterval(updateInterval_MS);
}

void MultiTimeHandler::getTime(){
    ntptimeIsNotWorking = !timeClient.forceUpdate();
    if(!ntptimeIsNotWorking && rtcIsNotWorking){    //si funciona NTP y no RTC se obtiene el tiempo de NTP
        hour = timeClient.getHours();
        minute = timeClient.getMinutes();
        dayOfTheWeek = timeClient.getDay();         //0 is Sunday
    }
    else if(rtcIsNotWorking){                       //Si no funciona ninguno se asegura que no se quede atrapado en un bucle infinto
        hour = 99;
        minute = 99;
        dayOfTheWeek = 99;
    }
    else{                                           //Si funciona RTC se obtiene el tiempo de RTC         
        now = rtc.now();
        hour = now.hour();
        minute = now.minute();
        dayOfTheWeek = now.dayOfTheWeek();
    }
}

void MultiTimeHandler::update(){
    updateTimeTask->handler();
}

void MultiTimeHandler::forceUpdate(){
    getTime();
}

uint8_t MultiTimeHandler::getHour(){
    return hour;
}

uint8_t MultiTimeHandler::getMinute(){
    return minute;
}

uint8_t MultiTimeHandler::getDayOfTheWeek(){
    return dayOfTheWeek;
}