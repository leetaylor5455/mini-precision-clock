#pragma once
#define BAT_FULL 4.2f
#define BAT_EMPT 2.8f


/*!
    @brief Convert ADC reading into voltage.
    @param reading ADC reading (0-1023)
*/
float voltage(int reading);

/*!
    @brief Convert battery voltage into state-of-charge (SOC).
    @param voltage Battery voltage
*/
int SOC(float voltage);