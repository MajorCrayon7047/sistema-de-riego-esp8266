#include "Task.h"
#include <Arduino.h>

Task::Task(TaskFunction callbackFunction, unsigned long intervalInMS, int iterations, bool enabled, TaskFunction onEnable, TaskFunction onDisable){     //Constructor. Si se incializa con 0 iteraciones se convierte en una tarea infinita
    this->callbackFunction = callbackFunction;
    this->interval = intervalInMS;
    this->maxIterations = iterations;
    this->onEnable = onEnable;
    this->enabled = enabled;

    this->actualTime = 0;
    this->timeReference = 0;
    this->finalTime = 0;
    this->currentIteration = 0;
    this->finished = false;
}

bool Task::isLastIteration(){
    return currentIteration == maxIterations - 1;
}

int Task::getInterval(){
    return interval;
}

int Task::getCurrentIteration(){
    return currentIteration;
}

int Task::isFinished(){
    return finished;
}

bool Task::isEnabled(){
    return enabled;
}

void Task::setOnEnable( TaskFunction onEnable ){
    this->onEnable = onEnable;
}

void Task::setInterval(unsigned long intervalInMS){
    this->interval = intervalInMS;
}

void Task::setIterations(int iterations){
    this->maxIterations = iterations;
}

void Task::enable(){
    if(onEnable != nullptr) onEnable();
    enabled = true;
    finished = false;
    calcNextTime();
    currentIteration = 0;
}

void Task::enableIfNot(){
    if(!enabled) enable();
}

void Task::disable(){
    if(onDisable != nullptr) onDisable();
    enabled = false;
    finished = true;
    currentIteration = 0;
}

void Task::setOnDisable( TaskFunction onDisable ){
    this->onDisable = onDisable;
}

void Task::handler(){
    if(enabled){
        actualTime = millis();
        if(actualTime >= finalTime){
            callbackFunction();
            currentIteration++;
            if(currentIteration==maxIterations && maxIterations != 0){
                disable();
            }
            else {
                calcNextTime();
            }
        }
    }
}

void Task::calcNextTime(){
    timeReference = millis();
    finalTime = timeReference + interval;
}