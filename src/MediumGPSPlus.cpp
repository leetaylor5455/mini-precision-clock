#include <MediumGPSPlus.h>

MediumGPSPlus::MediumGPSPlus() {
    valid = 0;
    offset = 0;
}

void MediumGPSPlus::printInfo()
{
    Serial.print(F("  Date/Time: "));

    if (valid) {
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

    else {
        // validate();
        Serial.print(F("0/0/0000 00:00:00.00"));
    }

    

    Serial.println();
}

void MediumGPSPlus::validate() {
    // Date comes through as 2080 prematurely
    // Janky
    if (date.isValid() && date.year() < 2080) {
        digitalWrite(LED_BUILTIN, HIGH);
        valid = 1;

        setOffset();
    } else {
        digitalWrite(LED_BUILTIN, LOW);
        valid = 0;
    }
}

void MediumGPSPlus::setOffset() {
    if (valid) {

        int y = date.year() - 2000;
        int x = (y + y/4 + 2) % 7; // remainder will identify which day of month

        // ************* Begins on first Sunday of Mar @ 1:00 AM ************
        if (date.month() == 3 && date.day() == (14 - x) && time.hour() >= 2) {
            offset = 1;
        }
        if (date.month() == 3 && date.day() > (14 - x) || date.month() > 3) {
            offset = 1;
        }
        // ************* Ends on last Sunday of Oct @ 2:00 AM ************
        if (date.month() == 10 && date.day() == (31 - x) && time.hour() >= 2) {
            offset = 0;
        }
        
        if (date.month() == 10 && date.day() > (31 - x) || date.month() > 10 || date.month() < 3) {
            offset = 0;
        }
    } else {
        offset = 0;
    }
    
}