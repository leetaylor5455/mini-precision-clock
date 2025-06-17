#include <LedControl.h>
#include <TinyGPSPlus.h>
#include <PreciseTime.h>

#define MS_SHOW 2000
#define MS_DP_SWITCH 150

class LEDDisplay: public LedControl {
    
    using LedControl::LedControl;

    public:
        /*!
            @brief Show time on LED display.
            @param dateTime precise time object
        */
        void showTime(PreciseTime dateTime);

        /*!
            @brief Show date on LED display.
            @param dateTime precise time object
        */
        void showDate(PreciseTime dateTime);

        /*!
            @brief Show voltage on LED display.
            @param voltage battery voltage
            @param SOC battery state-of-charge
        */
        void showBattery(float voltage, int SOC, int charging);

        /*!
            @brief Iterate the intensity index
        */
        void iterateIntensity();

        unsigned int idxIntensity;
        bool loading;
    private:
        int loadingIdx = 0;
        unsigned long tSinceDP;
        int DPTime[8] = {0, 0, 1, 0, 1, 0, 1, 0};
        int DPDate[8] = {0, 0, 0, 0, 1, 0, 1, 0};
        int intensities[3] = {1, 5, 15};
};