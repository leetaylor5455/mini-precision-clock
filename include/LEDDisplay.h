#include <LedControl.h>
#include <TinyGPSPlus.h>
#include <PreciseTime.h>

class LEDDisplay: public LedControl {
    
    using LedControl::LedControl;

    public:
        void showTime(PreciseTime dateTime);
};