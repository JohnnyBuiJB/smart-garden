#pragma once
#include <stdint.h>

// ── System ────────────────────────────────────────────────────────────────────
#define NUM_PLANTS  10

// ── GPIO — Relays (active LOW) ────────────────────────────────────────────────
#define PIN_PUMP        4
// Valve pins indexed 0–9, matching plant IDs from sensor nodes
static const int VALVE_PINS[NUM_PLANTS] = { 5, 18, 19, 21, 22, 23, 13, 15, 2, 0 };
// NOTE: GPIO0 (valve 9) requires a 10kΩ pull-up to 3.3V on the PCB.
//       Verify it is not pulled LOW during boot by the relay coil.

// ── GPIO — Monitoring ─────────────────────────────────────────────────────────
#define PIN_BATTERY_ADC  34   // ADC1_CH6, input-only pin — battery voltage divider

// ── Moisture thresholds (%) ───────────────────────────────────────────────────
// Plants are watered when their reported moisture falls below these values.
// Adjust per plant type (e.g. succulents prefer lower, herbs prefer higher).
static const float MOISTURE_THRESHOLD[NUM_PLANTS] = {
    40.0f, 40.0f, 40.0f, 40.0f, 40.0f,
    40.0f, 40.0f, 40.0f, 40.0f, 40.0f
};

// ── Watering duration (ms) ────────────────────────────────────────────────────
// How long to run the pump per plant per watering event.
static const uint32_t WATERING_DURATION_MS[NUM_PLANTS] = {
    5000, 5000, 5000, 5000, 5000,
    5000, 5000, 5000, 5000, 5000
};

// ── Timing ────────────────────────────────────────────────────────────────────
#define SLEEP_INTERVAL_US       (30ULL * 60 * 1000000)  // 30 minutes total cycle
#define RECEIVE_WINDOW_MS       60000   // stay awake to collect sensor packets
#define VALVE_OPEN_SETTLE_MS    100     // wait after opening valve before starting pump
#define PUMP_STOP_SETTLE_MS     200     // wait after stopping pump before closing valve

// ── Battery ───────────────────────────────────────────────────────────────────
#define BATTERY_DIVIDER_RATIO   2.0f
#define BATTERY_LOW_VOLTS       3.5f    // skip watering below this voltage
