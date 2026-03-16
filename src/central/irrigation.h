#pragma once
#include "../common/espnow_packet.h"
#include <stdint.h>

// Sets all relay GPIOs to HIGH (safe/open state). Call first in setup().
void irrigation_init();

// Waters a single plant: opens its valve, runs the pump, then closes the valve.
void irrigation_water_plant(uint8_t plant_id, uint32_t duration_ms);

// Iterates all plants; waters those with moisture below their threshold.
// Reads moisture from the provided readings array (indexed by plant_id).
void irrigation_run_cycle(const SensorPacket readings[], const bool received[]);

// Forces all relays to safe (open) state. Call before sleeping.
void irrigation_safe_state();
