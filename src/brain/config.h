#pragma once
#include <stdint.h>

// ── System ────────────────────────────────────────────────────────────────────
#define NUM_PLANTS  10

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID       "your_ssid"
#define WIFI_PASSWORD   "your_password"

// ── MQTT ──────────────────────────────────────────────────────────────────────
#define MQTT_BROKER     "192.168.1.x"   // Pi or laptop IP running Mosquitto
#define MQTT_PORT       1883

// ── GPIO — Relays (active LOW) ────────────────────────────────────────────────
#define PIN_PUMP        4
// Valve pins indexed 0–9, matching plant IDs from sensor nodes
static const int VALVE_PINS[NUM_PLANTS] = { 5, 18, 19, 21, 22, 23, 13, 15, 2, 0 };
// NOTE: GPIO0 (valve 9) requires a 10kΩ pull-up to 3.3V on the PCB.

// ── Timing ────────────────────────────────────────────────────────────────────
#define VALVE_OPEN_SETTLE_MS    100
#define PUMP_STOP_SETTLE_MS     200
