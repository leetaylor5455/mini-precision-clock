#include <MediumGPSPlus.h>

MediumGPSPlus::MediumGPSPlus()
{
    valid = 0;
}

void MediumGPSPlus::printTime()
{
    Serial.print(F("  Date/Time (UTC): "));
    Serial.print(date.day());
    Serial.print(F("/"));
    Serial.print(date.month());
    Serial.print(F("/"));
    Serial.print(date.year());
    Serial.print(F(" "));
    if (time.hour() < 10) Serial.print(F("0"));
    Serial.print(time.hour());
    Serial.print(F(":"));
    if (time.minute() < 10) Serial.print(F("0"));
    Serial.print(time.minute());
    Serial.print(F(":"));
    if (time.second() < 10) Serial.print(F("0"));
    Serial.print(time.second());
    Serial.print(F("."));
    if (time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(time.centisecond());
    Serial.println();
}

void MediumGPSPlus::validate()
{
    // Date comes through as 2080 prematurely
    // Janky
    valid = (date.isValid() && date.year() < 2080) ? 1 : 0;
}

bool MediumGPSPlus::hasFix()
{
    // A position fix means the module has decoded the full nav message,
    // including the week number, so the date can be trusted.
    // Without one, the time of day is right but the date may be weeks out.
    return location.isValid() && location.age() < 2000;
}

uint32_t MediumGPSPlus::unixTime()
{
    int year = date.year();       // e.g. 2025
    int month = date.month();     // 1-12
    int day = date.day();         // 1-31
    int hour = time.hour();       // 0-23
    int minute = time.minute();   // 0-59
    int second = time.second();   // 0-59

    // Days since 1970/01/01
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

    // UTC: daylight savings is applied at display time (see isBST)
    uint32_t totalSeconds =
        days * 86400UL +
        hour * 3600UL +
        minute * 60UL +
        second;

    return totalSeconds;
}
