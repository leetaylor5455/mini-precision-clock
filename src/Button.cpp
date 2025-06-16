#include <Button.h>
#include <Arduino.h>

Button::Button(int pin) 
{
    this->pin = pin;
}

void Button::init() 
{
    pinMode(pin, INPUT);

    state = HIGH;
    prevState = HIGH;
    DCwaiting = false;
    DConUp = false;
    singleOK = true;
    downTime = -1;
    upTime = -1;
    ignoreUp = false;
    waitForUp = false;
    holdEventPast = false;
    longHoldEventPast = false;
    
}

void Button::checkEvent() 
{
  
    event = 0;
    state = digitalRead(pin);

    // Button pressed down
    if (state == HIGH && prevState == LOW && (millis() - upTime) > MS_DEBOUNCE)
    {
        downTime = millis();
        ignoreUp = false;
        waitForUp = false;
        singleOK = true;
        holdEventPast = false;
        longHoldEventPast = false;
        if ((millis()-upTime) < MS_DCGAP && DConUp == false && DCwaiting == true)  { DConUp = true; }
        else { DConUp = false; }
        DCwaiting = false;
    }
    // Button released
    else if (state == LOW && prevState == HIGH && (millis() - downTime) > MS_DEBOUNCE)
    {        
        if (!ignoreUp)
        {
            upTime = millis();
            if (DConUp == false) { DCwaiting = true; }
            else
            {
                event = 2;
                DConUp = false;
                DCwaiting = false;
                singleOK = false;
            }
        }
    }
    // Test for normal click event: MS_DCGAP expired
    if (state == LOW && (millis()-upTime) >= MS_DCGAP && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
    {
        event = 1;
        DCwaiting = false;
    }
    // Test for hold
    if (state == HIGH && (millis() - downTime) >= MS_HOLD) {
        // Trigger "normal" hold
        if (!holdEventPast)
        {
            event = 3;
            waitForUp = true;
            ignoreUp = true;
            DConUp = false;
            DCwaiting = false;
            //downTime = millis();
            holdEventPast = true;
        }
        // Trigger "long" hold
        if ((millis() - downTime) >= MS_LONGHOLD)
        {
            if (!longHoldEventPast)
            {
                event = 4;
                longHoldEventPast = true;
            }
        }
    }
    prevState = state;
}
