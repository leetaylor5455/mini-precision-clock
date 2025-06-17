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

// #define MS_BETWEEN_SYNCS 1000*60*60 // Sync every hour
#define MS_BETWEEN_SYNCS 1000*60 // Sync every minute
#define MS_WAIT_GPS 30000 // Try GPS sync for 30 seconds

static const bool core1_disable_systick = true;
bool core1_separate_stack = true;
static const int RXPin = 1, TXPin = 0;
static const int batVPin = 29;
static const int chargingPin = 24;
static const PreciseTime zeroTime(2000, 1, 1);

// volatile bool fifo_write = false;

RTC_DS3231 rtc;
MediumGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
LEDDisplay display(19, 18, 17, 1);
Button batButton(14);
Button modButton(15);

void setup() 
{
    Serial.begin(115200);

    // while (!Serial);

    // Override default I2C pins for RTC
    Wire.setSDA(8);
    Wire.setSCL(9);
    Wire.begin();

    // ss.begin(9600);

    // delay(1000);

    batButton.init();
    modButton.init();

    // GP29 is battery voltage divider
    pinMode(batVPin, INPUT);
    pinMode(chargingPin, INPUT);

    // Turn off all digits in LED display
    for(int i = 0; i < display.getDeviceCount(); i++) 
    {
        display.shutdown(i, false); 
    } 

    while (!rtc.begin()) 
    {
        Serial.println("Couldn't find RTC");
        display.showTime(zeroTime);
        delay(100);
    }

}

void setup1() 
{
    ss.begin(9600);
}

void loop() 
{
    unsigned long tSinceSync;
    unsigned long tNow;
    unsigned long tSec = millis();
    unsigned long centis = 0;
    uint8_t sec = 0;

    DateTime rtcTime;
    PreciseTime preciseTime;
    PreciseTime timerStartTime = zeroTime;
    PreciseTime timerStopTime = zeroTime;
    PreciseTime timerCurrentTime = zeroTime;
    uint32_t timerSpentPaused = 0;

    uint32_t gpsUnixTime;

    static const int intensities[3] = {1, 5, 15};
    unsigned int idxIntensity;

    if (digitalRead(chargingPin)) { display.idxIntensity = 1; }
    else { display.idxIntensity = 0; }

    float batVoltage;

    bool showDate = false;
    bool timerMode = false;
    bool timerAlreadyStarted = false;
    bool timerRunning = false;
    bool syncing = true;

    while (true) 
    {
        tNow = millis();

        rtcTime = rtc.now();
        preciseTime.updateFromRTC(rtcTime);

        if (preciseTime.second() != sec)
        {
            sec = preciseTime.second();
            tSec = tNow;
        }

        batButton.checkEvent();
        modButton.checkEvent();
        display.setIntensity(0, display.idxIntensity);

        // Click MOD -> toggle between date and time view
        if (modButton.event == 1) { showDate = !showDate; }

        // Double click MOD -> toggle timer mode
        if (modButton.event == 2) { timerMode = !timerMode; }

        // Click BAT -> read battery status
        if (batButton.event == 1 && !timerMode) 
        {
            batVoltage = voltage(analogRead(batVPin));
            display.showBattery(batVoltage, SOC(batVoltage), digitalRead(chargingPin));
        }

        // Double click BAT -> change brightness
        if (batButton.event == 2) { display.iterateIntensity(); }    

        // Click BAT IN TIMER
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

        // Hold BAT in TIMER -> reset timer
        if (timerMode && batButton.event == 3) 
        {
            timerCurrentTime = zeroTime;
            timerSpentPaused = 0;
            timerAlreadyStarted = false;
            timerRunning = false;
        }

        // GPS sync due, or manual sync
        if (tNow - tSinceSync > MS_BETWEEN_SYNCS || modButton.event == 3) { syncing = true; }
        if (syncing) 
        {
            rp2040.resumeOtherCore();
            display.loading = true;

            if (rp2040.fifo.available() > 0) 
            {
                gpsUnixTime = rp2040.fifo.pop();
                
                // If it's good, sync and send core to sleep
                if (gpsUnixTime > 0) 
                {
                    rtc.adjust(DateTime(gpsUnixTime));
                    rp2040.idleOtherCore();
                    rp2040.fifo.clear();
                    tSinceSync = millis();
                    display.loading = false;
                    syncing = false;
                    gpsUnixTime = 0;
                    // Serial.println("Synced with GPS");
                }
            }
        }

        // Count centiseconds
        preciseTime.cs = (tNow - tSec) / 10;
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
        else // Time / date mode
        {
            if (showDate) { display.showDate(preciseTime); }
            else { display.showTime(preciseTime); }
        }
        // Serial.println(PreciseTime(preciseTime.unixtime() - timerStartTime.unixtime()).unixtime());
        delay(20);
    }
}

void loop1()
{
    uint8_t sec;

    while (ss.available() > 0) 
    { 
        if (gps.encode(ss.read())) 
        {
            if (gps.valid && gps.unixTime() != gps.previousUnixTime) // On valid connection, sync on seconds flip
            { 
                sec = gps.time.second();
                while (gps.encode(ss.read()) && gps.time.second() == sec);

                gps.setOffset();
                // gps.printTime();
                gps.previousUnixTime = gps.unixTime();
                rp2040.fifo.push(gps.unixTime());

                gps.printTime();

                // return;
            } 
            else { gps.validate(); }
        }
        if (millis() > 5000 && gps.charsProcessed() < 10) 
        {
            Serial.println(F("No GPS detected: check wiring."));
        }
    }
}
