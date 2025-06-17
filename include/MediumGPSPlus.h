#pragma once
#include <TinyGPSPlus.h>

class MediumGPSPlus: public TinyGPSPlus 
{
    private:
        int offset;

    public:
        MediumGPSPlus();
        /*!
            @brief Basic check on validity (not 2080).
        */
        void validate();

        /*!
            @brief Calculate daylight savings offset.
        */
        void setOffset();
        
        /*!
            @brief Print time to serial monitor.
        */
        void printTime();
        
        /*!
            @brief Convert time into unix epoch in seconds.
        */
        uint32_t unixTime();
        int valid;
        uint32_t previousUnixTime = 0;
};