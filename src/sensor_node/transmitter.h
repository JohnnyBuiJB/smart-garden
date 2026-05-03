#pragma once
#include "../common/espnow_packet.h"
#include <stdbool.h>

void transmitter_init();

// Sends packet to the Brain via ESP-NOW.
// Retries up to ESPNOW_RETRIES times. Returns true if ACK received.
bool transmitter_send(const SensorPacket *pkt);

void transmitter_deinit();
