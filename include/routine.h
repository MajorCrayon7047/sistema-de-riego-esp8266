#ifndef ROUTINE_CLASS_H
#define ROUTINE_CLASS_H

#include "Task.h"

#include <ArduinoJson.h>
#include "MultiTimeHandler.h"

class Routine {
    private:
        Task* routineTask;
        uint8_t currentPin, numberOfPins;

        MultiTimeHandler* time;

        void routineHandler(void);
        void routineOnEnable(void);
        void routineOnDisable(void);
        

    public:
        bool routineAuto, routineState;
        JsonDocument configRutina;

        Routine();
        void begin(void);
        bool loadConfig(bool autoModeState = false);
        
        void enable(void);
        void disable(void);
        void handler(void);
};

#endif // ROUTINE_CLASS_H