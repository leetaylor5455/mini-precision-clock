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

void setup1() { ss.begin(9600); }

void loop() 
{
    uint32_t millisNow;
    uint32_t millisAtSecChange = millis();
    uint8_t currentSec = 0;

    DateTime rtcTime;
    PreciseTime now;

    float batteryVoltage;

    ChronoState chronograph;
    GPSSyncState gpsSync;
    Mode mode = Mode::TIME;

    while (true) 
    {
        rtcTime = rtc.now();
        now.updateFromRTC(rtcTime);

        millisNow = millis();

        if (now.second() != currentSec)
        {
            currentSec = now.second();
            millisAtSecChange = millisNow;
        }
        
        display.setIntensity(0, display.idxIntensity);
        display.loading = gpsSync.inProgress;
        display.showPreciseTime = gpsSync.synced;

        batButton.checkEvent();
        modButton.checkEvent();

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
                case Mode::CHRONO:
                    chronograph.handleClick(now);
                    break;
                default:
                    break;
            }
        }

        if (modButton.event == DOUBLE_CLICK) { mode = (mode == Mode::TIME) ? Mode::CHRONO : Mode::TIME; }

        if (batButton.event == CLICK) 
        {
            batteryVoltage = voltage(analogRead(BAT_V_PIN));
            display.showBattery(batteryVoltage, SOC(batteryVoltage), digitalRead(CHARGING_PIN));
        }
        
        if (batButton.event == DOUBLE_CLICK) { display.iterateIntensity(); }   

        if (batButton.event == HOLD) { chronograph.reset(); }

        gpsSync.due = (millisNow - gpsSync.millisAtLastSync) > MS_BETWEEN_SYNCS;

        if (gpsSync.due || modButton.event == HOLD)
        {
            if (!gpsSync.inProgress) { gpsSync.millisAtSyncStart = millisNow; }
            gpsSync.inProgress = true;
        }

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
            if (millisNow - gpsSync.millisAtSyncStart > MS_WAIT_GPS) { gpsSync.setUnsynced(millisNow); }
        }

        now.cs = (millisNow - millisAtSecChange) / 10;
        now.setTimeArr();
        now.setDateArr();

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
                    delay(1000 + MS_CALIB_OFFSET); // Delay 1s + calibration ms and send time + 1s
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