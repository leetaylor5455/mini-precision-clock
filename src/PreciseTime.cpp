#include <PreciseTime.h>

void PreciseTime::updateFromRTC(DateTime now) {
    hh = now.hour();
    mm = now.minute();
    ss = now.second();
}   

void PreciseTime::setTimeArr() {

    // uint8_t tempShortArr[4] = { hh, mm, ss, cs };
    uint8_t tempShortArr[4] = { cs, ss, mm, hh };

    // Returns first digit of number, or zero if single digit
    auto firstDigit = [](int num) {
        if (num == 0) { return 0; }
        return num % 10;
    };

    // Returns second digit of number, or first digit if single digit
    auto secondDigit = [](int num) {
        if (num == 0) { return 0; }
        return (num - num % 10) / 10;
    };

    for (int i = 0; i < sizeof(tempShortArr); i++) {
        timeArr[2*i] = firstDigit(tempShortArr[i]);
        timeArr[2*i+1] = secondDigit(tempShortArr[i]);
    }
}