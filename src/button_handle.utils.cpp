#include "button_handle_utils.h"

void handleModButtonClick(Mode *mode, ChronoState *chronograph, PreciseTime now)
{
    switch (*mode)
    {
        case Mode::TIME:
            *mode = Mode::DATE;
            break;
        case Mode::DATE:
            *mode = Mode::TIME;
            break;
        case Mode::CHRONO:
            chronograph->handleClick(now);
            break;
        default:
            break;
    }
}

void handleBatButtonClick(LEDDisplay display)
{
    float batteryVoltage = voltage(analogRead(BAT_V_PIN));
    display.showBattery(batteryVoltage, SOC(batteryVoltage), digitalRead(CHARGING_PIN));
}