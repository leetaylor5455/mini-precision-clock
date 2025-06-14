#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

#include <gps_utils.h>
#include <MediumGPSPlus.h>
#include <LedControl.h>
#include <LEDDisplay.h>


RTC_DS3231 rtc;

static const int RXPin = 1, TXPin = 0;
static const uint32_t GPSBaud = 9600;

// TinyGPSPlus gps;
MediumGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
// LedControl display(19, 18, 17, 1);
LEDDisplay display(19, 18, 17, 1);

void setup()
{
	Serial.begin(115200);
	ss.begin(GPSBaud);
	pinMode(LED_BUILTIN, OUTPUT);

    // Set I2C pins for RTC
    Wire.setSDA(8);
    Wire.setSCL(9);
    Wire.begin();

    // Turn off all digits
    for(int i = 0; i < display.getDeviceCount(); i++) {
        display.shutdown(i, false); 
    } 

    while (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        // Serial.flush();
        delay(100);
    }

    // if (rtc.lostPower()) {
    //     Serial.println("RTC lost power, let's set the time!");
    //     // When time needs to be set on a new device, or after a power loss, the
    //     // following line sets the RTC to the date & time this sketch was compiled
    //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     // This line sets the RTC with an explicit date & time, for example to set
    //     // January 21, 2014 at 3am you would call:
    //     // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    // }
}

void loop()
{

    unsigned long Tsync;
    unsigned long Tref;

    DateTime rtcTime;
    PreciseTime preciseTime;
    uint8_t sec = 0;
    unsigned long tSec = millis();
    unsigned long centis = 0;

    while (true) {
        rtcTime = rtc.now();

        preciseTime.updateFromRTC(rtcTime);

        if (preciseTime.second() != sec) {
            sec = preciseTime.second();
            tSec = millis();
        }

        // Count centiseconds
        preciseTime.cs = (millis() - tSec) / 10;

        preciseTime.setTimeArr();

        display.showTime(preciseTime);

        // Serial.print(preciseTime.hour(), DEC);
        // Serial.print(':');
        // Serial.print(preciseTime.minute(), DEC);
        // Serial.print(':');
        // Serial.print(preciseTime.second(), DEC);
        // Serial.print('.');
        // Serial.print(preciseTime.cs, DEC);
        // Serial.println();
        delay(20);
    }

    // while (true) {
    //     while (!ss.available() > 0) {};
    //     while (!gps.encode(ss.read())) {};

    //     while (!gps.valid) { gps.validate(); }
    //     gps.setOffset();

    //     Tref = gps.time.value();
    //     Tsync = millis();
        
    //     while (millis() - Tsync < 300000) {

    //     }
    // }
    

    // while (ss.available() > 0) {
    //     if (gps.encode(ss.read())) {

    //         // Wait for lock
    //         while (!gps.valid) { gps.validate(); }
    //         Tsync = millis();



    //         // gps.printInfo();
    //         gps.setOffset();

    //         // display.setDigit(0, (byte)1, 1, 1);
    //         display.setTime(gps.time);
    //         display.displayTime();
    //         // sleep_ms(1000);

            
    //     }

    //     if (millis() > 5000 && gps.charsProcessed() < 10) {
    //         Serial.println(F("No GPS detected: check wiring."));
    //         while(true);
    //     }
    // }
}

