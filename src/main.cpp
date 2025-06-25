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

void setup1() 
{
    ss.begin(9600);
}

void loop() 
{
    uint32_t millisNow;
    uint32_t millisSecChange = millis();
    uint8_t currentSec = 0;

    DateTime rtcTime;
    PreciseTime now;

    float batteryVoltage;

    ChronoState chronograph;
    GPSSyncState gpsSync;
    Mode mode = Mode::TIME;

    while (true) 
    {
        millisNow = millis();

        rtcTime = rtc.now();
        now.updateFromRTC(rtcTime);

        // Track milliseconds since seconds changes
        if (now.second() != currentSec)
        {
            currentSec = now.second();
            millisSecChange = millisNow;
        }
        
        display.setIntensity(0, display.idxIntensity);
        display.loading = gpsSync.inProgress;
        display.showPreciseTime = gpsSync.synced;

        batButton.checkEvent();
        modButton.checkEvent();

        // Click MOD -> toggle between date and time view (do nothing in chronograph)
        if (modButton.event == CLICK) 
        {
            switch (mode)
            {
                case Mode::TIME:
                    mode = Mode::DATE;
                    break;
                case Mode::DATE:
                    mode = Mode::TIME;
                    break;
                default:
                    break;
            }
        }

        // Double click MOD -> toggle timer mode
        if (modButton.event == DOUBLE_CLICK) { mode = (mode == Mode::TIME) ? Mode::CHRONO : Mode::TIME; }

        // Click BAT in time mode -> read battery status
        if (batButton.event == CLICK && mode != Mode::CHRONO) 
        {
            batteryVoltage = voltage(analogRead(BAT_V_PIN));
            display.showBattery(batteryVoltage, SOC(batteryVoltage), digitalRead(CHARGING_PIN));
        } 

        // Click BAT in chronograph mode
        if (batButton.event == CLICK && mode == Mode::CHRONO)
        {
            // Button pressed to start/resume chronograph
            if (!chronograph.running)
            {
                // Start chronograph if not already started
                if (!chronograph.started) { chronograph.start(now); } 
                // Chrono is being resumed, accumulate time spent paused
                else { chronograph.spentPaused += (now.unixtime() - chronograph.stopTime.unixtime()); }
            }
            // Button pressed to pause time
            else { chronograph.stopTime = now; }

            chronograph.running = !chronograph.running;
        }
        
        if (batButton.event == DOUBLE_CLICK) { display.iterateIntensity(); }   

        // Hold BAT in timer mode -> reset timer
        if (mode == Mode::CHRONO && batButton.event == HOLD) { chronograph.reset(); }

        // Make sync due when enough time elapsed since last sync
        gpsSync.due = (millisNow - gpsSync.millisLastSynced) > MS_BETWEEN_SYNCS;

        // GPS sync due after timeout or manual sync
        if (gpsSync.due || modButton.event == HOLD)
        {
            if (!gpsSync.inProgress) { gpsSync.millisSyncStart = millisNow; }
            gpsSync.inProgress = true;
        }

        // Sync RTC with GPS time (core 1)
        if (gpsSync.inProgress) 
        {
            rp2040.resumeOtherCore(); // Wake up core 1 to get GPS time

            if (rp2040.fifo.available() > 0) 
            {
                gpsSync.gpsUnixTime = rp2040.fifo.pop();

                if (!gpsSync.firstSync) 
                {
                    rtc.adjust(DateTime(gpsSync.gpsUnixTime));
                    rp2040.idleOtherCore();
                    gpsSync.setSynced(millisNow);
                }
                else { gpsSync.firstSync = false; }
                
                rp2040.fifo.clear();
            }

            // Timeout sync attempt
            if (millisNow - gpsSync.millisSyncStart > MS_WAIT_GPS) { gpsSync.setUnsynced(millisNow); }
        }

        // Count centiseconds
        now.cs = (millisNow - millisSecChange) / 10;
        now.setTimeArr();
        now.setDateArr();

        // Display according to current mode
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

void loop1()
{
    uint8_t currentSec;

    while (ss.available() > 0) 
    {
        if (gps.encode(ss.read())) 
        {
            if (gps.valid && gps.unixTime() != gps.previousUnixTime)
            { 
                currentSec = gps.time.second();
                while (gps.encode(ss.read()) && gps.time.second() == currentSec); // Wait for second to flip

                gps.setOffset();

                // Don't push if it's the first sync, because it holds old value on first sync
                if (gps.unixTime() > gps.previousUnixTime && gps.previousUnixTime != 0) 
                { 
                    delay(1000 + MS_CALIB_OFFSET); // Delay 1s + calibration and send time + 1s
                    rp2040.fifo.push(gps.unixTime() + 1);
                    gps.previousUnixTime = 0; 
                }
                else { gps.previousUnixTime = gps.unixTime(); }
                
                #ifdef DEBUG
                gps.printTime();
                #endif
            } 
            else { gps.validate(); }
        }
        if (millis() > 5000 && gps.charsProcessed() < 10) 
        {
            #ifdef DEBUG
            Serial.println(F("No GPS detected: check wiring."));
            #endif
        }
    }
}