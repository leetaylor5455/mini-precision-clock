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