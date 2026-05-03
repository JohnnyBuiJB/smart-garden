#pragma once
#include "../common/espnow_packet.h"
#include <stdbool.h>
#include <stdint.h>

// Starts ESP-NOW in station mode and registers the receive callback.
void receiver_init();

// Blocks for up to timeout_ms, collecting packets from all sensor nodes.
// Returns the number of unique plant IDs received.
int receiver_collect(uint32_t timeout_ms);

// Returns the last received packet for the given plant_id.
// Returns false if no packet has been received for that plant.
bool receiver_get(uint8_t plant_id, SensorPacket *out);

void receiver_deinit();
