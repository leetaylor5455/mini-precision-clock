#include <Arduino.h>

// Delays
constexpr uint32_t MS_BETWEEN_SYNCS = 1000UL * 60 * 10; // Sync every 10 minutes
constexpr uint32_t MS_WAIT_GPS = 10000;                 // Try GPS sync for 10 seconds
constexpr uint32_t MS_CALIB_OFFSET = -155;                   // Constant ms offset calibration

// Pins
constexpr int RX_PIN = 1;
constexpr int TX_PIN = 0;
constexpr int BAT_V_PIN = 29;
constexpr int CHARGING_PIN = 24;
constexpr int CLK_PIN = 19;
constexpr int CS_PIN  = 18;
constexpr int DIN_PIN = 17;
constexpr int NUM_DEVICES = 1;
constexpr int BAT_BUTTON_PIN = 14;
constexpr int MOD_BUTTON_PIN = 15;
constexpr int I2C_SDA = 8;
constexpr int I2C_SCL = 9;