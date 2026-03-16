#pragma once
#include <stdint.h>

// ── Identity ──────────────────────────────────────────────────────────────────
// Set a unique value 0–9 per physical node before flashing.
#define PLANT_ID  0

// ── GPIO ──────────────────────────────────────────────────────────────────────
#define PIN_SENSOR_ADC    1   // ADC1_CH1 — analog output of capacitive sensor
#define PIN_SENSOR_POWER  2   // P-FET gate: HIGH = sensor OFF, LOW = sensor ON
#define PIN_BATTERY_ADC   3   // ADC1_CH3 — battery voltage divider (100k/100k)
#define PIN_STATUS_LED    8   // Onboard LED (disable in production to save power)

// ── Central controller ESP-NOW MAC ───────────────────────────────────────────
// Replace with the actual MAC address of the central ESP32 after reading it
// via Serial.println(WiFi.macAddress()) on first boot.
#define CENTRAL_MAC  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

// ── Timing ────────────────────────────────────────────────────────────────────
#define SLEEP_INTERVAL_US   (30ULL * 60 * 1000000)  // 30 minutes
#define SENSOR_SETTLE_MS    500   // time to wait after powering sensor before reading
#define ADC_SAMPLES         10    // oversample count for stable ADC reads
#define ESPNOW_TIMEOUT_MS   3000  // max wait for ACK per send attempt
#define ESPNOW_RETRIES      3     // send attempts before giving up

// ── ADC ───────────────────────────────────────────────────────────────────────
// Calibration defaults (raw ADC counts at 11dB attenuation, 12-bit).
// Override via NVS after running the calibration procedure.
#define SENSOR_DRY_RAW      2800  // reading in completely dry air
#define SENSOR_WET_RAW      1200  // reading with probe submerged in water

// Battery voltage divider: two equal resistors → V_adc = V_bat / 2
// 12-bit ADC, Vref ≈ 3.3V → 1 LSB = 3.3 / 4096 V
#define BATTERY_DIVIDER_RATIO  2.0f
