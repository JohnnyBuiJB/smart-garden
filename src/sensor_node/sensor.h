#pragma once
#include <stdint.h>

void sensor_init();
void sensor_power_on();
void sensor_power_off();

// Returns raw 12-bit ADC count (averaged over ADC_SAMPLES reads).
int  sensor_read_raw();

// Converts raw ADC count to moisture percentage using calibration values.
// Reads dry/wet calibration from NVS if available, falls back to config.h defaults.
float sensor_to_percent(int raw);

// Reads battery voltage in volts using the onboard voltage divider.
float sensor_battery_voltage();
