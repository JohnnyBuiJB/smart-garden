#pragma once
#include <stdbool.h>
#include <stdint.h>

void  power_init();
float power_battery_voltage();
bool  power_is_low_battery();

// Enters deep sleep for the given microseconds.
void power_deep_sleep(uint64_t sleep_us);
