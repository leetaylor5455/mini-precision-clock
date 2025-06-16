#include <battery_utils.h>

float voltage(int reading)
{
    return reading * 3.0f * (3.3f / 1024.0f);
}

int SOC(float voltage) 
{
    int SOC =  100 * ((voltage - BAT_EMPT) / (BAT_FULL - BAT_EMPT)); 

    if (SOC > 100) { return 100; }
    return SOC; 
}