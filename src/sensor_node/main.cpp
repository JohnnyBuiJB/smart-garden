#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"
#include "sensor.h"
#include "transmitter.h"
#include "power.h"
#include "../common/espnow_packet.h"

// Uptime counter survives deep sleep but resets on power-off
static RTC_DATA_ATTR uint32_t s_wake_count = 0;

void setup() {
    Serial.begin(115200);
    delay(100);

    bool cold_boot = (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER);
    if (cold_boot) {
        s_wake_count = 0;
        Serial.printf("[NODE %d] Cold boot\n", PLANT_ID);
    }
    s_wake_count++;
    Serial.printf("[NODE %d] Wake #%lu\n", PLANT_ID, (unsigned long)s_wake_count);

    // ── 1. Read soil moisture ─────────────────────────────────────────────────
    sensor_init();
    sensor_power_on();
    delay(SENSOR_SETTLE_MS);

    int   raw         = sensor_read_raw();
    float moisture    = sensor_to_percent(raw);
    float battery_v   = sensor_battery_voltage();

    sensor_power_off();

    Serial.printf("[NODE %d] moisture=%.1f%% raw=%d bat=%.2fV\n",
                  PLANT_ID, moisture, raw, battery_v);

    // ── 2. Transmit via ESP-NOW ───────────────────────────────────────────────
    SensorPacket pkt;
    pkt.plant_id     = PLANT_ID;
    pkt.moisture_pct = moisture;
    pkt.battery_v    = battery_v;
    pkt.uptime_s     = s_wake_count * (SLEEP_INTERVAL_US / 1000000ULL);

    transmitter_init();
    bool ok = transmitter_send(&pkt);
    transmitter_deinit();

    if (!ok) {
        Serial.println("[NODE] TX failed after all retries");
    }

    // ── 3. Deep sleep ─────────────────────────────────────────────────────────
    Serial.printf("[NODE %d] sleeping %llu s\n", PLANT_ID, SLEEP_INTERVAL_US / 1000000ULL);
    power_deep_sleep(SLEEP_INTERVAL_US);
}

void loop() {
    // Unreachable — device always deep sleeps before returning from setup()
}
