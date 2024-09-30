#ifndef ROUTINE_H
#define ROUTINE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

extern JsonDocument configRutina;
extern uint8_t currentPin, numberOfPins;
extern Scheduler taskHandler;
extern Task routineHandlerTask;
extern bool routineAuto, routineState;

void initRoutineConfig(void);
bool loadRoutineConfig(bool autoModeState = false);
void routineHandler(void);
bool routineOnEnable(void);
void routineOnDisable(void);
void autoRoutineHandler(void);
void enableRoutine(void);
void disableRoutine(void);
void executeTaskHandler(void);


#endif // ROUTINE_H