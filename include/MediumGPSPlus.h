#pragma once
#include <TinyGPSPlus.h>

class MediumGPSPlus: public TinyGPSPlus
{
    public:
        MediumGPSPlus();
        /*!
            @brief Basic check on validity (not 2080).
        */
        void validate();

        /*!
            @brief True if there is a recent position fix, meaning the date
            (not just the time of day) can be trusted.
        */
        bool hasFix();

        /*!
            @brief Print time to serial monitor.
        */
        void printTime();
        
        /*!
            @brief Convert time into unix epoch in seconds (UTC).
        */
        uint32_t unixTime();
        int valid;
        uint32_t previousUnixTime = 0;
};