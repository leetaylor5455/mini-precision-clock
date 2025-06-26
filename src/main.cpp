#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

#include "config.h"
#include "MediumGPSPlus.h"
#include "LedControl.h"
#include "LEDDisplay.h"
#include "Button.h"
#include "battery_utils.h"
#include "button_handle_utils.h"
#include "sync_utils.h"
#include "state_types.h"

constexpr bool DEBUG = false;

RTC_DS3231 rtc;
MediumGPSPlus gps;
SoftwareSerial ss(RX_PIN, TX_PIN);
LEDDisplay display(CLK_PIN, CS_PIN, DIN_PIN, NUM_DEVICES);
Button batButton(BAT_BUTTON_PIN);
Button modButton(MOD_BUTTON_PIN);

void setup() 
{
    Serial.begin(115200);

    Wire.setSDA(I2C_SDA);
    Wire.setSCL(I2C_SCL);
    Wire.begin();

    batButton.init();
    modButton.init();

    pinMode(BAT_V_PIN, INPUT);
    pinMode(CHARGING_PIN, INPUT);

    display.shutdown(0, false); // Turn off all digits in display

    while (!rtc.begin()) { display.showWord((char*)"HELP    "); }
}

void setup1() { ss.begin(9600); }

void loop() 
{
    uint32_t millisNow;
    DateTime rtcTime;
    PreciseTime now;

    ChronoState chronograph;
    GPSSyncState gpsSync;

    Mode mode = Mode::TIME;

    while (true) 
    {
        rtcTime = rtc.now();
        now.updateFromRTC(rtcTime);
        millisNow = millis();
        now.updatePrecise(millisNow);
        
        display.setIntensity(0, display.idxIntensity);
        display.loading = gpsSync.inProgress;
        display.showPreciseTime = gpsSync.synced;

        batButton.checkEvent();
        modButton.checkEvent();

        if (modButton.event == CLICK) { handleModButtonClick(&mode, &chronograph, now); }
        if (modButton.event == DOUBLE_CLICK) { mode = (mode == Mode::TIME) ? Mode::CHRONO : Mode::TIME; }
        if (modButton.event == HOLD || gpsSync.due) { gpsSync.handleSyncDue(millisNow); }

        if (batButton.event == CLICK) { handleBatButtonClick(display); }
        if (batButton.event == DOUBLE_CLICK) { display.iterateIntensity(); }
        if (batButton.event == HOLD) { chronograph.reset(); }

        gpsSync.due = (millisNow - gpsSync.millisAtLastSync) > MS_BETWEEN_SYNCS;
        if (gpsSync.inProgress) { handleSyncCore0(&rtc, &gpsSync, millisNow); }

        switch (mode)
        {
            case Mode::TIME:
                display.showTime(now);
                break;
            case Mode::CHRONO:
                if (chronograph.running) { chronograph.update(now); }
                display.showTime(chronograph.currentTime); 
                break;
            case Mode::DATE:
                display.showDate(now);
                break;
            default:
                display.showWord((char*)"HELP    ");
                break;
        }
    }
}

void loop1() { handleSyncCore1(&ss, &gps); }