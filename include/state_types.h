#pragma once
#include "PreciseTime.h"

struct ChronoState 
{
    bool started = false;
    bool running = false;
    uint32_t spentPaused = 0;
    PreciseTime startTime = zeroTime;
    PreciseTime stopTime = zeroTime;
    PreciseTime currentTime = zeroTime;

    void start(PreciseTime now);
    void update(PreciseTime now);
    void reset();
};

struct GPSSyncState 
{
    bool due;
    bool inProgress = true;
    bool synced = false;
    bool firstSync = true;
    uint32_t millisLastSynced;
    uint32_t millisSyncStart;
    uint32_t gpsUnixTime;

    void setSynced(uint32_t msNow);

    void setUnsynced(uint32_t msNow);
};

enum class Mode 
{
    TIME,
    DATE,
    CHRONO
};

enum ButtonEvent 
{
    CLICK = 1,
    DOUBLE_CLICK = 2,
    HOLD = 3
};