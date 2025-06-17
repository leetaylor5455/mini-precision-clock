#pragma once
#include <RTClib.h>

class PreciseTime: public DateTime {
    using DateTime::DateTime;

    public:
        // Store time in array for easy iteration over digits
        uint8_t timeArr[8];
        uint8_t dateArr[8];
        
        uint8_t cs;

        /*!
            @brief Expand time into array of single digits.
        */
        void setTimeArr();

        /*!
            @brief Expand date into array of single digits.
        */
        void setDateArr();

        /*!
            @brief Copy time from RTC
        */
        void updateFromRTC(DateTime rtcTime);


        /*!
            @brief Print the time to the Serial monitor
        */
       void print();
};