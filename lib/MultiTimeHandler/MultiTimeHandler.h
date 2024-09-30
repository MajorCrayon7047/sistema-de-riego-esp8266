#ifndef MULTI_TIME_HANDLER_H
#define MULTI_TIME_HANDLER_H

#include <RTClib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Task.h"

class MultiTimeHandler {
    private:
        Task* updateTimeTask;
        void getTime(void);

        //NTP time
        WiFiUDP ntpUDP;
        NTPClient timeClient;
        bool ntptimeIsNotWorking;

        //RTC stuff
        RTC_DS3231 rtc;
        DateTime now;
        bool rtcIsNotWorking;
    
    public:
        //time
        uint8_t hour, minute, dayOfTheWeek;

        MultiTimeHandler(unsigned long updateInterval_MS = 60000);
        void begin(void);
        void setupTime(const char* date = __DATE__, const char* time = __TIME__);           //setup RTC time
        void setUpdateInterval(unsigned long updateInterval_MS);
        void update(void);
        void forceUpdate(void);         //force update time
        uint8_t getHour(void);
        uint8_t getMinute(void);
        uint8_t getDayOfTheWeek(void);  //0 is Sunday
};

#endif // MULTI_TIME_HANDLER_H