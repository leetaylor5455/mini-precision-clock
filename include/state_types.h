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

    /*!
        @brief Handle the chronograph function on button click
        @param now current precise time
    */
    void handleClick(PreciseTime now);

    /*!
        @brief Set the states to start the chronograph
        @param now current precise time
    */
    void start(PreciseTime now);

    /*!
        @brief Update the chronograph time state using the current time
        @param now current precise time
    */
    void update(PreciseTime now);

    /*!
        @brief Reset all the chronograph states
    */
    void reset();
};

struct GPSSyncState 
{
    bool due;
    bool inProgress = true;
    bool synced = false;
    bool firstSync = true;
    uint32_t millisAtLastSync;
    uint32_t millisAtSyncStart;
    uint32_t gpsUnixTime;

    /*!
        @brief Set states accordingly when sync is due or button pressed
        @param millisNow current milliseconds
    */
    void handleSyncDue(uint32_t millisNow);

    /*!
        @brief Set states accordingly when sync is successful
        @param millisNow current milliseconds
    */
    void setSynced(uint32_t millisNow);

    /*!
        @brief Set states accordingly when sync is unsuccessful
        @param millisNow current milliseconds
    */
    void setUnsynced(uint32_t millisNow);
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