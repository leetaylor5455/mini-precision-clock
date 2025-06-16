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
static const int RXPin = 1, TXPin = 0;
static const int batVPin = 29;
static const int chargingPin = 24;

RTC_DS3231 rtc;
MediumGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
LEDDisplay display(19, 18, 17, 1);
Button batButton(14);
Button modButton(15);

void setup() 
{
    Serial.begin(115200);

    // Override default I2C pins for RTC
    Wire.setSDA(8);
    Wire.setSCL(9);
    Wire.begin();

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
        display.showTime(PreciseTime((uint32_t)0));
        delay(100);
    }

}

void setup1() 
{
    ss.begin(9600);
}

void loop() 
{
    unsigned long tSinceSync = -MS_BETWEEN_SYNCS; // Sync on boot
    unsigned long tNow;
    unsigned long tSec = millis();
    unsigned long centis = 0;
    uint8_t sec = 0;

    DateTime rtcTime;
    PreciseTime preciseTime;
    uint32_t gpsUnixTime;

    static const int intensities[3] = {1, 5, 15};
    unsigned int idxIntensity = 1;
    
    float batVoltage;

    while (true) 
    {
        batButton.checkEvent();
        modButton.checkEvent();

        // Double click BAT -> change brightness
        if (batButton.event == 2)
        {
            if (idxIntensity == 2) { idxIntensity = 0; }
            else { idxIntensity++; }
        }
        display.setIntensity(0, intensities[idxIntensity]);

        if (batButton.event == 1) 
        {
            batVoltage = voltage(analogRead(batVPin));
            if (digitalRead(chargingPin)) { Serial.println("Charging"); }

            display.showBattery(batVoltage, SOC(batVoltage));
            delay(2000);
        }

        tNow = millis();

        // Sync with GPS if due
        if (tNow - tSinceSync > MS_BETWEEN_SYNCS || 
            (batButton.event == 3 && modButton.event == 3) ||
            (batButton.event == 3 && modButton.event == 1) ||
            (batButton.event == 2 && modButton.event == 3)
        ) 
        {
            rp2040.resumeOtherCore();
            if (rp2040.fifo.available() > 0) {
               
                // Get value from FIFO
                gpsUnixTime = rp2040.fifo.pop();

                // If it's good, sync and send core to sleep
                if (gpsUnixTime > 0) {

                    rtc.adjust(DateTime(gpsUnixTime));
                    rp2040.idleOtherCore();
                    tSinceSync = millis();
                    rp2040.fifo.clear();

                    Serial.println("Synced with GPS");
                }
            }
        }

        rtcTime = rtc.now();

        preciseTime.updateFromRTC(rtcTime);

        if (preciseTime.second() != sec) 
        {
            sec = preciseTime.second();
            tSec = tNow;
        }

        // Count centiseconds
        preciseTime.cs = (tNow- tSec) / 10;
        preciseTime.setTimeArr();

        display.showTime(preciseTime);
        // preciseTime.print();

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
            if (gps.valid) 
            { 
                // On valid connection, sync on seconds flip
                sec = gps.time.second();
                while (gps.encode(ss.read()) && gps.time.second() == sec);

                gps.setOffset();
                // gps.printTime();
                rp2040.fifo.push((uint32_t)gps.unixTime());
                return;

            } 
            else { gps.validate(); }
        }

        if (millis() > 5000 && gps.charsProcessed() < 10) 
        {
            Serial.println(F("No GPS detected: check wiring."));
            // while(true);
        }
        
    }
}

