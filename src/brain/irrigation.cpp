#include "irrigation.h"
#include "config.h"
#include <Arduino.h>

void irrigation_init() {
    // Initialise pump pin
    pinMode(PIN_PUMP, OUTPUT);
    digitalWrite(PIN_PUMP, HIGH);  // relay open = pump off

    // Initialise all valve pins
    for (int i = 0; i < NUM_PLANTS; i++) {
        pinMode(VALVE_PINS[i], OUTPUT);
        digitalWrite(VALVE_PINS[i], HIGH);  // relay open = valve closed
    }
}

void irrigation_water_plant(uint8_t plant_id, uint32_t duration_ms) {
    if (plant_id >= NUM_PLANTS) return;

    Serial.printf("[IRR] watering plant %d for %lu ms\n",
                  plant_id, (unsigned long)duration_ms);

    // 1. Open valve
    digitalWrite(VALVE_PINS[plant_id], LOW);
    delay(VALVE_OPEN_SETTLE_MS);

    // 2. Start pump
    digitalWrite(PIN_PUMP, LOW);
    delay(duration_ms);

    // 3. Stop pump
    digitalWrite(PIN_PUMP, HIGH);
    delay(PUMP_STOP_SETTLE_MS);

    // 4. Close valve
    digitalWrite(VALVE_PINS[plant_id], HIGH);
}

void irrigation_run_cycle(const SensorPacket readings[], const bool received[]) {
    for (int i = 0; i < NUM_PLANTS; i++) {
        if (!received[i]) {
            Serial.printf("[IRR] plant %d: no reading, skipping\n", i);
            continue;
        }

        float moisture = readings[i].moisture_pct;
        if (moisture < MOISTURE_THRESHOLD[i]) {
            Serial.printf("[IRR] plant %d: moisture %.1f%% < threshold %.1f%%, watering\n",
                          i, moisture, MOISTURE_THRESHOLD[i]);
            irrigation_water_plant(i, WATERING_DURATION_MS[i]);
        } else {
            Serial.printf("[IRR] plant %d: moisture %.1f%% ok\n", i, moisture);
        }
    }
}

void irrigation_safe_state() {
    digitalWrite(PIN_PUMP, HIGH);
    for (int i = 0; i < NUM_PLANTS; i++) {
        digitalWrite(VALVE_PINS[i], HIGH);
    }
}
