#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

#include <MediumGPSPlus.h>
#include <LedControl.h>
#include <LEDDisplay.h>
#include <Button.h>
#include <battery_utils.h>

#define MS_BETWEEN_SYNCS 1000*60 // Sync every minute
#define MS_WAIT_GPS 10000 // Try GPS sync for 10 seconds
#define MS_CALIB_OFFSET -155 // Constant ms offset calibration

static const int RXPin = 1, TXPin = 0;
static const int batVPin = 29;
static const int chargingPin = 24;
static const PreciseTime zeroTime(2000, 1, 1);

RTC_DS3231 rtc;
MediumGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
LEDDisplay display(19, 18, 17, 1);
Button batButton(14);
Button modButton(15);

void setup() 
{
    Serial.begin(115200);

    Wire.setSDA(8);
    Wire.setSCL(9);
    Wire.begin();

    batButton.init();
    modButton.init();

    pinMode(batVPin, INPUT);
    pinMode(chargingPin, INPUT);

    display.shutdown(0, false); // Turn off all digits in display

    while (!rtc.begin()) { display.showWord((char*)"HELP    "); }
}

void setup1() 
{
    ss.begin(9600);
    ss.flush();
}

void loop() 
{
    unsigned long tSynced;
    unsigned long tSyncingStart;
    unsigned long tNow;
    unsigned long tReceiveGPS;
    unsigned long tSecChange = millis();
    unsigned long centis = 0;
    uint8_t sec = 0;

    DateTime rtcTime;
    PreciseTime preciseTime;
    PreciseTime timerStartTime = zeroTime;
    PreciseTime timerStopTime = zeroTime;
    PreciseTime timerCurrentTime = zeroTime;
    uint32_t timerSpentPaused = 0;

    uint32_t gpsUnixTime;

    if (digitalRead(chargingPin)) { display.idxIntensity = 1; }
    else { display.idxIntensity = 1; } // Optional intensity for when boot on battery power

    float batVoltage;

    bool showDate = false;
    bool timerMode = false;
    bool timerAlreadyStarted = false;
    bool timerRunning = false;
    
    bool syncDue;
    bool syncing = true;
    bool synced = false;
    bool firstSync = true;

    while (true) 
    {
        tNow = millis();

        rtcTime = rtc.now();
        preciseTime.updateFromRTC(rtcTime);

        if (preciseTime.second() != sec)
        {
            sec = preciseTime.second();
            tSecChange = tNow;
        }
        
        display.setIntensity(0, display.idxIntensity);
        display.loading = syncing;
        display.showPreciseTime = synced;

        batButton.checkEvent();
        modButton.checkEvent();

        // Click MOD -> toggle between date and time view
        if (modButton.event == 1) { showDate = !showDate; }

        // Double click MOD -> toggle timer mode
        if (modButton.event == 2) { timerMode = !timerMode; }

        // Click BAT -> read battery status
        if (batButton.event == 1 && !timerMode) 
        {
            batVoltage = voltage(analogRead(batVPin));
            display.showBattery(voltage(analogRead(batVPin)), SOC(batVoltage), digitalRead(chargingPin));
        }

        // Double click BAT -> change brightness
        if (batButton.event == 2) { display.iterateIntensity(); }    

        // Click BAT in timer mode
        if (timerMode && batButton.event == 1)
        {
            if (!timerRunning) // Button pressed to start/resume timer
            {
                if (!timerAlreadyStarted) // No timer previsously exists, mark start point
                { 
                    timerStartTime = preciseTime;
                    timerAlreadyStarted = true;
                } 
                // Time is being resumed, accumulate time spent paused
                else { timerSpentPaused += (preciseTime.unixtime() - timerStopTime.unixtime()); }
            }
            // Button pressed to pause time
            else { timerStopTime = preciseTime; }
            timerRunning = !timerRunning;
        }

        // Hold BAT in timer mode -> reset timer
        if (timerMode && batButton.event == 3) 
        {
            timerCurrentTime = zeroTime;
            timerSpentPaused = 0;
            timerAlreadyStarted = false;
            timerRunning = false;
        }

        syncDue = (tNow - tSynced) > MS_BETWEEN_SYNCS;

        // GPS sync due after timeout or manual sync
        if (syncDue || modButton.event == 3)
        {
            if (!syncing) { tSyncingStart = tNow; }
            syncing = true;
        }

        if (syncing) 
        {
            rp2040.resumeOtherCore();

            if (rp2040.fifo.available() > 0) 
            {
                gpsUnixTime = rp2040.fifo.pop();
                
                if (!firstSync) // Bugs out on first syncs -> janky fix for now
                {
                    rtc.adjust(DateTime(gpsUnixTime));
                    rp2040.idleOtherCore();
                    tSynced = tNow;
                    syncing = false;
                    synced = true;
                    gpsUnixTime = 0;
                    // firstSync = true; // Uncomment to double sync every time
                    // Serial.println("Synced with GPS");
                }
                else { firstSync = false; }
                rp2040.fifo.clear();
            }

            if (tNow - tSyncingStart > MS_WAIT_GPS) // Timeout sync attempt
            { 
                syncing = false;
                synced = false;
                tSynced = tNow;
            }
        }

        // Count centiseconds
        preciseTime.cs = (tNow - tSecChange) / 10;
        preciseTime.setTimeArr();
        preciseTime.setDateArr();

        if (timerMode)
        {
            if (timerRunning)
            {
                timerCurrentTime = PreciseTime(
                    zeroTime.unixtime()
                    + preciseTime.unixtime() 
                    - timerStartTime.unixtime()
                    - timerSpentPaused
                );
                timerCurrentTime.cs = preciseTime.cs;
                timerCurrentTime.setTimeArr();  
            }
            display.showTime(timerCurrentTime);            
        }
        else if (showDate) { display.showDate(preciseTime); }
        else { display.showTime(preciseTime); }
        // delay(20);
    }
}

void loop1()
{
    uint8_t sec;

    while (ss.available() > 0) 
    {
        if (gps.encode(ss.read())) 
        {
            if (gps.valid && gps.unixTime() != gps.previousUnixTime)
            { 
                sec = gps.time.second();
                while (gps.encode(ss.read()) && gps.time.second() == sec); // Wait for second to flip

                gps.setOffset();

                // Don't push if it's the first sync, because it holds old value on first sync
                if (gps.unixTime() > gps.previousUnixTime && gps.previousUnixTime != 0) 
                { 
                    delay(1000 + MS_CALIB_OFFSET); // Delay 1s + calibration and send time + 1s
                    rp2040.fifo.push(gps.unixTime() + 1);
                    gps.previousUnixTime = 0; 
                }
                else { gps.previousUnixTime = gps.unixTime(); }
                
                // gps.printTime();
            } 
            else { gps.validate(); }
        }
        if (millis() > 5000 && gps.charsProcessed() < 10) 
        {
            Serial.println(F("No GPS detected: check wiring."));
        }
    }
}