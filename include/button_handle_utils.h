#pragma once
#include "config.h"
#include "LEDDisplay.h"
#include "battery_utils.h"
#include "state_types.h"
#include "PreciseTime.h"

/*!
    @brief Switches between time/date display, or passes on to chronograph handle.
    @param mode current clock mode
    @param chronograph chronograph state
    @param now PreciseTime instance for passing into chronograph
*/
void handleModButtonClick(Mode *mode, ChronoState *chronograph, PreciseTime now);

/*!
    @brief Calculate voltage and SOC and display.
    @param display LED display instance
*/
void handleBatButtonClick(LEDDisplay display);