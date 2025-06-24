#include <LEDDisplay.h>

void LEDDisplay::showTime(PreciseTime dateTime) 
{
    int digitEnd = 0;
    if (!showPreciseTime) // Hide precise display if not synced
    {
        setChar(0, 0, '-', false);
        setChar(0, 1, '-', false);
        digitEnd = 2;
    }
    
    for (int i = sizeof(dateTime.timeArr) - 1; i >= digitEnd; i--) 
    {
        if (loading)
        {
            setDigit(0, i, (byte)dateTime.timeArr[i], loadingIdx == i ? true : false); 

            if (millis() - tSinceDP > MS_DP_SWITCH)
            {
                if (loadingIdx == 0) { loadingIdx = 7; }
                else { loadingIdx--; }
                tSinceDP = millis();
            }
        }
        else
        {
            setDigit(0, i, (byte)dateTime.timeArr[i], DPTime[i]);
        }
    }
}

void LEDDisplay::showDate(PreciseTime dateTime)
{
    for (int i = sizeof(dateTime.dateArr) - 1; i >= 0; i--) 
    {
        setDigit(0, i, (byte)dateTime.dateArr[i], DPDate[i]);
    }
}

void LEDDisplay::showBattery(float voltage, int SOC, int charging) 
{
    clearDisplay(0);

    // Separate across decimal point
    int vInt = (int)voltage;
    int vDec = (int)((voltage - vInt) * 10);
    
    int mod = -1;
    int i = 0;
    
    // Extract digits: check if SOC > 0, perform the modulo, and then divide by 10
    for (; SOC > 0; SOC /= 10, i++)
    {  
        mod = SOC % 10;
        setDigit(0, i, (byte)mod, false);
    }

    // Display voltage on left
    setDigit(0, 7, (byte)vInt, true);
    setDigit(0, 6, (byte)vDec, false);
    
    if (charging)
    {
        for (int j = 5; j >= i-1; j--)
        {
            delay(MS_SHOW / (7-i));
            setChar(0, j, '-', false);
        }
        
    }
    else { delay(MS_SHOW); }
}

void LEDDisplay::iterateIntensity()
{
    if (idxIntensity == 2) { idxIntensity = 0; }
    else { idxIntensity++; }

    // setIntensity(0, intensities[idxIntensity]);
}

