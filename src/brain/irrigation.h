#pragma once
#include <stdint.h>

// Sets all relay GPIOs to HIGH (safe/open state). Call first in setup().
void irrigation_init();

// Opens valve, runs pump for duration_ms, then closes valve.
void irrigation_water_plant(uint8_t plant_id, uint32_t duration_ms);

// Forces all relays to safe (open) state.
void irrigation_safe_state();
