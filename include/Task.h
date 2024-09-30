#ifndef TASK_H
#define TASK_H

#include <functional>

class Task {

    private:
        typedef std::function<void(void)> TaskFunction;
        unsigned long interval;
        unsigned long timeReference;
        unsigned long finalTime;
        unsigned long actualTime;
        
        TaskFunction callbackFunction;
        TaskFunction onEnable;
        TaskFunction onDisable;
        int maxIterations;
        int currentIteration;
        bool enabled;
        bool finished;

        void calcNextTime(void);

    public:
        Task(TaskFunction callbackFunction, unsigned long interval, int iterations, bool enabled = false, TaskFunction onEnable = nullptr, TaskFunction onDisable = nullptr);
        bool isLastIteration(void);
        int getInterval(void);
        int getCurrentIteration(void);
        int isFinished(void);
        bool isEnabled(void);
        void setOnEnable( TaskFunction onEnable );
        void setOnDisable( TaskFunction onDisable );
        void setInterval(unsigned long interval);
        void setIterations(int iterations);
        void enable(void);
        void enableIfNot(void);
        void disable(void);
        void handler(void);
};

#endif  // TASK_H