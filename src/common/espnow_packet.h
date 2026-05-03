#pragma once
#include <stdint.h>

// Packet transmitted from each sensor node to the Brain via ESP-NOW.
// Both firmware images must include this header to ensure struct layout matches.
struct SensorPacket {
    uint8_t  plant_id;       // 0–9
    float    moisture_pct;   // 0.0–100.0 (calibrated)
    float    battery_v;      // node LiPo voltage (e.g. 3.7)
    uint32_t uptime_s;       // seconds since node last cold-booted (for diagnostics)
};
