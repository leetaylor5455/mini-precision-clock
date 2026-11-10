#include "state_types.h"
#include "config.h"

#include "MediumGPSPlus.h"
#include "SoftwareSerial.h"

/*!
    @brief Whether UK daylight savings (BST) is in effect
    @param utc time in UTC
*/
bool isBST(const DateTime &utc);

/*!
    @brief Combine the RTC's date with the GPS's time of day, choosing
    the day nearest the RTC so syncs around midnight stay correct
    @param rtcUnix RTC unix time
    @param gpsUnix GPS unix time (date part not trusted)
*/
uint32_t mergeTimeOfDay(uint32_t rtcUnix, uint32_t gpsUnix);

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