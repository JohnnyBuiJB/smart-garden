#pragma once
#include <stdint.h>

// Enters deep sleep for the given number of microseconds.
// All GPIO state is lost; setup() runs again on wake.
void power_deep_sleep(uint64_t sleep_us);
