#include "state_types.h"
#include "config.h"

#include "MediumGPSPlus.h"
#include "SoftwareSerial.h"

/*!
    @brief Core 0 side of GPS sync handling
    @param rtc RTC instance
    @param gpsSync GPS sync state
    @param millisNow milliseconds now
*/
void handleSyncCore0(RTC_DS3231 *rtc, GPSSyncState *gpsSync, uint32_t millisNow);

/*!
    @brief Core 1 side of GPS sync handling
    @param ss Software Serial instance
    @param gps MediumGPSPlus instance
*/
void handleSyncCore1(SoftwareSerial *ss, MediumGPSPlus *gps);