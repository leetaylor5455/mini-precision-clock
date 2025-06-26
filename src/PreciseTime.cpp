#include "PreciseTime.h"
#include <Arduino.h>

void PreciseTime::updateFromRTC(DateTime now) 
{
    d = now.day();
    m = now.month();
    yOff = now.year() - 2000;
    hh = now.hour();
    mm = now.minute();
    ss = now.second();
}   

void PreciseTime::updatePrecise(uint32_t millisNow)
{
    if (ss != currentSec)
    {
        currentSec = ss;
        millisAtSecChange = millisNow;
    }

    cs = (millisNow - millisAtSecChange) / 10;
    
    setTimeArr();
    setDateArr();
}

void PreciseTime::setTimeArr() 
{
    // uint8_t tempShortArr[4] = { hh, mm, ss, cs };
    uint8_t tempShortArr[4] = { cs, ss, mm, hh };

    // Returns first digit of number, or zero if single digit
    auto firstDigit = [](int num) 
    {
        if (num == 0) { return 0; }
        return num % 10;
    };

    // Returns second digit of number, or first digit if single digit
    auto secondDigit = [](int num) 
    {
        if (num == 0) { return 0; }
        return (num - num % 10) / 10;
    };

    for (int i = 0; i < sizeof(tempShortArr); i++) 
    {
        timeArr[2*i] = firstDigit(tempShortArr[i]);
        timeArr[2*i+1] = secondDigit(tempShortArr[i]);
    }
}

void PreciseTime::setDateArr() 
{
    // uint8_t tempShortArr[4] = { hh, mm, ss, cs };
    // uint8_t tempShortArr[4] = { d, m, 20, yOff };
    uint8_t tempShortArr[4] = { yOff, (uint8_t)20, m, d };

    // Returns first digit of number, or zero if single digit
    auto firstDigit = [](int num) 
    {
        if (num == 0) { return 0; }
        return num % 10;
    };

    // Returns second digit of number, or first digit if single digit
    auto secondDigit = [](int num) 
    {
        if (num == 0) { return 0; }
        return (num - num % 10) / 10;
    };

    for (int i = 0; i < sizeof(tempShortArr); i++) 
    {
        dateArr[2*i] = firstDigit(tempShortArr[i]);
        dateArr[2*i+1] = secondDigit(tempShortArr[i]);
    }
}

void PreciseTime::print() 
{
    Serial.print(day());
    Serial.print(F("/"));
    Serial.print(month());
    Serial.print(F("/"));
    Serial.print(year());
    Serial.print(F(" "));
    if (hour() < 10) { Serial.print(F("0")); }
    Serial.print(hour());
    Serial.print(F(":"));
    if (minute() < 10) { Serial.print(F("0")); }
    Serial.print(minute());
    Serial.print(F(":"));
    if (second() < 10) { Serial.print(F("0")); }
    Serial.print(second());
    Serial.print(F("."));
    if (cs < 10) { Serial.print(F("0")); }
    Serial.print(cs);

    Serial.println();
}

const PreciseTime zeroTime(2000, 1, 1);