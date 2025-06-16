#include <LEDDisplay.h>

void LEDDisplay::showTime(PreciseTime dateTime) 
{
    for (int i = sizeof(dateTime.timeArr) - 1; i >= 0; i--) 
    {
        setDigit(0, i, (byte)dateTime.timeArr[i], i % 2 == 0 ? true : false);
    }
}

void LEDDisplay::showBattery(float voltage, int SOC) 
{
    clearDisplay(0);

    // Separate across decimal point
    int vInt = (int)voltage;
    int vDec = (int)((voltage - vInt) * 10);
    
    int mod = -1;
    
    // Extract digits: check if SOC > 0, perform the modulo, and then divide by 10
    for (int i = 0; SOC > 0; SOC /= 10, i++)
    {  
        mod = SOC % 10;
        setDigit(0, i, (byte)mod, false);
    }

    // Display voltage on left
    setDigit(0, 7, (byte)vInt, true);
    setDigit(0, 6, (byte)vDec, false);
    
}

