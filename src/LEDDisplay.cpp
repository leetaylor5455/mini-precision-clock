#include <LEDDisplay.h>

// void LEDDisplay::setTime(TinyGPSTime GPStime) {
//     time[0] = (int)GPStime.centisecond();
//     time[1] = (int)GPStime.second();
//     time[2] = (int)GPStime.minute();
//     time[3] = (int)GPStime.hour();

// }

// void LEDDisplay::displayTime() {

//     auto firstDigit = [](int time) {
//         if (time == 0) { return 0; }
//         return time % 10;
//     };

//     auto secondDigit = [](int time) {
//         if (time == 0) { return 0; }
//         return (time - time % 10) / 10;
//     };

//     for (int i = 0; i < 4; i++) {
//         setDigit(0, 2*i, (byte)firstDigit(time[i]), i == 0 ? false : true);
//         setDigit(0, 2*i+1, (byte)secondDigit(time[i]), 0);
//     }

//     // for (int i = 0; i < 8; i+=2) {
//     //     setDigit(0, i, (byte)i, 0);
//     //     setDigit(0, i+1, (byte)(i+1), 0);
//     // }
// }

void LEDDisplay::showTime(PreciseTime dateTime) {

    for (int i = sizeof(dateTime.timeArr) - 1; i >= 0; i--) {
        setDigit(0, i, (byte)dateTime.timeArr[i], i % 2 == 0 ? true : false);
    }
}

