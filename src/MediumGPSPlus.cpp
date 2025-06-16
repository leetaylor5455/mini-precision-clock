#include <MediumGPSPlus.h>

MediumGPSPlus::MediumGPSPlus() 
{
    valid = 0;
    offset = 0;
}

void MediumGPSPlus::printTime()
{

    Serial.print(F("  Date/Time: "));

    if (true) 
    {
        Serial.print(date.day());
        Serial.print(F("/"));
        Serial.print(date.month());
        Serial.print(F("/"));
        Serial.print(date.year());
        Serial.print(F(" "));
        if (time.hour() + offset < 10) Serial.print(F("0"));
        Serial.print(time.hour() + offset);
        Serial.print(F(":"));
        if (time.minute() < 10) Serial.print(F("0"));
        Serial.print(time.minute());
        Serial.print(F(":"));
        if (time.second() < 10) Serial.print(F("0"));
        Serial.print(time.second());
        Serial.print(F("."));
        if (time.centisecond() < 10) Serial.print(F("0"));
        Serial.print(time.centisecond());
    }
    else 
    {
        Serial.print(F("0/0/0000 00:00:00.00"));
    }

    Serial.println();
}

void MediumGPSPlus::validate() 
{
    // Date comes through as 2080 prematurely
    // Janky
    if (date.isValid() && date.year() < 2080) 
    {
        // digitalWrite(LED_BUILTIN, HIGH);
        valid = 1;
    } 
    else 
    {
        // digitalWrite(LED_BUILTIN, LOW);
        valid = 0;
    }
}

void MediumGPSPlus::setOffset() 
{
    if (valid) 
    {
        int y = date.year() - 2000;
        int x = (y + y/4 + 2) % 7; // remainder will identify which day of month

        // ************* Begins on first Sunday of Mar @ 1:00 AM ************
        if (date.month() == 3 && date.day() == (14 - x) && time.hour() >= 2) 
        {
            offset = 1;
        }
        if (date.month() == 3 && date.day() > (14 - x) || date.month() > 3) 
        {
            offset = 1;
        }
        // ************* Ends on last Sunday of Oct @ 2:00 AM ************
        if (date.month() == 10 && date.day() == (31 - x) && time.hour() >= 2) 
        {
            offset = 0;
        }
        
        if (date.month() == 10 && date.day() > (31 - x) || date.month() > 10 || date.month() < 3) 
        {
            offset = 0;
        }
    } 
    else 
    {
        offset = 0;
    }
    
}

uint32_t MediumGPSPlus::unixTime() 
{
    int year = date.year();       // e.g. 2025
    int month = date.month();     // 1-12
    int day = date.day();         // 1-31
    int hour = time.hour();       // 0-23
    int minute = time.minute();   // 0-59
    int second = time.second();   // 0-59

    // Days since 1970/01/01 using Zeller's Congruence (basic version)
    const int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    unsigned long days = 0;

    for (int y = 1970; y < year; y++) 
    {
        days += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
    }

    for (int m = 1; m < month; m++) 
    {
        days += days_in_month[m - 1];
        if (m == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) 
        {
            days += 1; // leap year February
        }
    }

    days += day - 1; // current day

    uint32_t totalSeconds =
        days * 86400UL +
        (hour+offset) * 3600UL +
        minute * 60UL +
        second;

    return totalSeconds;
}