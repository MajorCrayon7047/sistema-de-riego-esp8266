#ifndef ROUTINE_CLASS_H
#define ROUTINE_CLASS_H

#include <TaskScheduler.h>
#include <ArduinoJson.h>

class Routine {
    private:
        Scheduler* taskHandler;
        Task* routineHandlerTask;
        uint8_t currentPin, numberOfPins;

        void routineHandler(void);
        bool routineOnEnable(void);
        void routineOnDisable(void);

    public:
        bool routineAuto, routineState;
        JsonDocument configRutina;

        Routine(Scheduler* taskHandler);
        void initRoutineConfig(void);
        bool loadRoutineConfig(bool autoModeState = false);
        
        void enableRoutine(void);
        void disableRoutine(void);
        void executeTaskHandler(void);
        void autoRoutineHandler(void);
};

#endif // ROUTINE_CLASS_H