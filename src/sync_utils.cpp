#include "sync_utils.h"

// Set by core 1 just before it pushes a time onto the FIFO
static volatile bool gpsDateTrusted = false;

bool isBST(const DateTime &utc)
{
    // UK: BST from 01:00 UTC on the last Sunday of March
    //     until 01:00 UTC on the last Sunday of October
    uint8_t m = utc.month();
    if (m < 3 || m > 10) { return false; }
    if (m > 3 && m < 10) { return true; }

    // dayOfTheWeek(): 0 = Sunday, so this is the date of the last Sunday
    uint8_t lastSunday = 31 - DateTime(utc.year(), m, 31).dayOfTheWeek();
    bool pastChange = utc.day() > lastSunday || (utc.day() == lastSunday && utc.hour() >= 1);

    return (m == 3) ? pastChange : !pastChange;
}

uint32_t mergeTimeOfDay(uint32_t rtcUnix, uint32_t gpsUnix)
{
    // RTC's date with GPS's time of day
    uint32_t merged = (rtcUnix - rtcUnix % 86400UL) + (gpsUnix % 86400UL);

    // Pick whichever day is nearest the RTC, so syncs either side of midnight land on the right date
    if ((int32_t)(merged - rtcUnix) > 43200) { merged -= 86400UL; }
    if ((int32_t)(rtcUnix - merged) > 43200) { merged += 86400UL; }

    return merged;
}

void handleSyncCore0(RTC_DS3231 *rtc, GPSSyncState *gpsSync, uint32_t millisNow)
{
    rp2040.resumeOtherCore(); // Wake up core 1 to get GPS time

    if (rp2040.fifo.available() > 0)
    {
        gpsSync->gpsUnixTime = rp2040.fifo.pop();
        bool dateTrusted = gpsDateTrusted;

        if (!gpsSync->firstSync)
        {
            uint32_t newTime = gpsSync->gpsUnixTime;

            // Without a position fix the GPS date can be weeks out, so only take
            // the time of day and keep the RTC's date (unless the RTC has lost it)
            DateTime rtcNow = rtc->now();
            bool rtcDateLost = rtc->lostPower() || rtcNow.year() < 2025;
            if (!dateTrusted && !rtcDateLost) { newTime = mergeTimeOfDay(rtcNow.unixtime(), newTime); }

            rtc->adjust(DateTime(newTime));
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

                // Don't push if it's the first sync, because it holds old value on first sync
                if (gps->unixTime() > gps->previousUnixTime && gps->previousUnixTime != 0)
                {
                    bool fix = gps->hasFix(); // Check before the delay so the fix age is fresh
                    delay(1000 + MS_CALIB_OFFSET); // Delay 1s + calibration ms and send time + 1s
                    gpsDateTrusted = fix;
                    rp2040.fifo.push(gps->unixTime() + 1);
                    gps->previousUnixTime = 0;
                }
                else { gps->previousUnixTime = gps->unixTime(); }

                #ifdef DEBUG
                gps->printTime();
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
