#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"
#include "receiver.h"
#include "irrigation.h"
#include "storage.h"
#include "power.h"
#include "../common/espnow_packet.h"

// Cycle counter survives deep sleep
static RTC_DATA_ATTR uint32_t s_cycle = 0;

// Track elapsed active time so sleep duration compensates for wake duration
static unsigned long s_boot_ms = 0;

void setup() {
    s_boot_ms = millis();

    Serial.begin(115200);
    delay(100);

    // ── 1. Safe relay state (must be first to avoid accidental activation) ────
    irrigation_init();

    bool cold_boot = (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER);
    s_cycle++;
    Serial.printf("[CENTRAL] cycle=%lu cold_boot=%d\n",
                  (unsigned long)s_cycle, (int)cold_boot);

    // ── 2. Power monitoring ───────────────────────────────────────────────────
    power_init();
    float bat_v = power_battery_voltage();
    Serial.printf("[CENTRAL] battery=%.2fV\n", bat_v);

    // ── 3. Collect sensor readings via ESP-NOW ────────────────────────────────
    storage_init();
    receiver_init();
    receiver_collect(RECEIVE_WINDOW_MS);
    receiver_deinit();

    // ── 4. Build readings snapshot ────────────────────────────────────────────
    SensorPacket readings[NUM_PLANTS];
    bool         received[NUM_PLANTS];
    for (int i = 0; i < NUM_PLANTS; i++) {
        received[i] = receiver_get(i, &readings[i]);
        if (received[i]) {
            storage_clear_missed(i);
        } else {
            storage_inc_missed(i);
            Serial.printf("[CENTRAL] plant %d missed (total=%d)\n",
                          i, storage_get_missed(i));
        }
    }

    // ── 5. Irrigate if battery is sufficient ─────────────────────────────────
    if (power_is_low_battery()) {
        Serial.printf("[CENTRAL] low battery (%.2fV), skipping irrigation\n", bat_v);
    } else {
        irrigation_run_cycle(readings, received);
    }

    // ── 6. Ensure safe relay state before sleep ───────────────────────────────
    irrigation_safe_state();

    // ── 7. Deep sleep — compensate for active time to stay in sync with nodes ─
    unsigned long active_ms  = millis() - s_boot_ms;
    uint64_t      sleep_us   = SLEEP_INTERVAL_US;
    if (active_ms * 1000ULL < sleep_us) {
        sleep_us -= (uint64_t)active_ms * 1000ULL;
    }
    Serial.printf("[CENTRAL] active=%lu ms, sleeping %llu s\n",
                  active_ms, sleep_us / 1000000ULL);

    power_deep_sleep(sleep_us);
}

void loop() {
    // Unreachable — device always deep sleeps before returning from setup()
}
