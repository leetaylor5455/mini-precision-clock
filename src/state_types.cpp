#include "state_types.h"

void ChronoState::reset()
{
    started = false;
    running = false;
    spentPaused = 0;
    startTime = stopTime = currentTime = zeroTime;
}

void ChronoState::update(PreciseTime now) 
{
    currentTime = PreciseTime(
        zeroTime.unixtime()
        + now.unixtime() 
        - startTime.unixtime()
        - spentPaused
    );
    currentTime.cs = now.cs;
    currentTime.setTimeArr();  
}

void ChronoState::start(PreciseTime now)
{
    startTime = now;
    started = true;
}

void GPSSyncState::setSynced(uint32_t tNow) 
{
    millisLastSynced = tNow;
    inProgress = false;
    synced = true;
    gpsUnixTime = 0;
}

void GPSSyncState::setUnsynced(uint32_t tNow)
{
    inProgress = false;
    synced = false;
    millisLastSynced = tNow;
}