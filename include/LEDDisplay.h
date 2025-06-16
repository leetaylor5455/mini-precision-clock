#include <LedControl.h>
#include <TinyGPSPlus.h>
#include <PreciseTime.h>

class LEDDisplay: public LedControl {
    
    using LedControl::LedControl;

    public:
        /*!
            @brief Show time on LED display.
            @param dateTime precise time object
        */
        void showTime(PreciseTime dateTime);

        /*!
            @brief Show voltage on LED display.
            @param voltage battery voltage
            @param SOC battery state-of-charge
        */
        void showBattery(float voltage, int SOC);
};