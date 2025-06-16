#pragma once
#include <Arduino.h>

#define MS_DEBOUNCE 20   // Debounce (press) milliseconds
#define MS_HOLD 1000     // Hold milliseconds
#define MS_DCGAP 250     // Double click gap milliseconds
#define MS_LONGHOLD 3000 // Long hold milliseconds

class Button {
    public:
        /*!
            @brief Create button instance with pin number and set Arduino pinMode.
        */
        Button(int pin);

        /*!
            @brief Set Arduino pinMode.
        */
        void init();

        /*!
            @brief Check what kind of event is happening with the button.
        */
        void checkEvent();

        int event;

    private:
        int pin;

        bool state;   // value read from button
        bool prevState;  // buffered value of the button's previous state
        bool DCwaiting;  // whether we're waiting for a double click (down)
        bool DConUp;     // whether to register a double click on next release, or whether to wait and click
        bool singleOK;    // whether it's OK to do a single click
        long downTime;         // time the button was pressed down
        long upTime;           // time the button was released
        bool ignoreUp;   // whether to ignore the button release because the click+hold was triggered
        bool waitForUp;        // when held, whether to wait for the up event
        bool holdEventPast;    // whether or not the hold event happened already
        bool longHoldEventPast;// whether or not the long hold event happened already
    
};