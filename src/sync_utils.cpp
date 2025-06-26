#include "sync_utils.h"

void handleSyncCore0(RTC_DS3231 *rtc, GPSSyncState *gpsSync, uint32_t millisNow)
{
    rp2040.resumeOtherCore(); // Wake up core 1 to get GPS time

    if (rp2040.fifo.available() > 0) 
    {
        gpsSync->gpsUnixTime = rp2040.fifo.pop();

        if (!gpsSync->firstSync) 
        {
            rtc->adjust(DateTime(gpsSync->gpsUnixTime));
            rp2040.idleOtherCore();
            gpsSync->setSynced(millisNow);
        }
        else { gpsSync->firstSync = false; }
        
        rp2040.fifo.clear();
    }

    // Timeout sync attempt
    if (millisNow - gpsSync->millisAtSyncStart > MS_WAIT_GPS) { gpsSync->setUnsynced(millisNow); }
}

void handleSyncCore1(SoftwareSerial *ss, MediumGPSPlus *gps)
{
    uint8_t currentSec;

    while (ss->available() > 0) 
    {
        if (gps->encode(ss->read())) 
        {
            if (gps->valid && gps->unixTime() != gps->previousUnixTime)
            { 
                currentSec = gps->time.second();
                while (gps->encode(ss->read()) && gps->time.second() == currentSec); // Wait for second to flip

                gps->setOffset();

                // Don't push if it's the first sync, because it holds old value on first sync
                if (gps->unixTime() > gps->previousUnixTime && gps->previousUnixTime != 0) 
                { 
                    delay(1000 + MS_CALIB_OFFSET); // Delay 1s + calibration ms and send time + 1s
                    rp2040.fifo.push(gps->unixTime() + 1);
                    gps->previousUnixTime = 0; 
                }
                else { gps->previousUnixTime = gps->unixTime(); }
                
                #ifdef DEBUG
                gps.printTime();
                #endif
            } 
            else { gps->validate(); }
        }
        if (millis() > 5000 && gps->charsProcessed() < 10) 
        {
            #ifdef DEBUG
            Serial.println(F("No GPS detected: check wiring."));
            #endif
        }
    }
}