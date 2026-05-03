#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"
#include "sensor.h"
#include "transmitter.h"
#include "power.h"
#include "../common/espnow_packet.h"

static RTC_DATA_ATTR uint32_t s_wake_count = 0;

static void led_blink(int times, int on_ms, int off_ms) {
    for (int i = 0; i < times; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(on_ms);
        digitalWrite(PIN_STATUS_LED, LOW);
        if (i < times - 1) delay(off_ms);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(100);

    bool cold_boot = (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER);
    if (cold_boot) {
        s_wake_count = 0;
        Serial.printf("[NODE %d] Cold boot\n", PLANT_ID);
        // 3 quick blinks = cold boot
        led_blink(3, 100, 100);
    } else {
        Serial.printf("[NODE %d] Wake #%lu\n", PLANT_ID, (unsigned long)s_wake_count + 1);
        // 1 blink = timer wake
        led_blink(1, 100, 0);
    }
    s_wake_count++;

    // ── 1. Read soil moisture ─────────────────────────────────────────────────
    // Solid on while reading sensor
    digitalWrite(PIN_STATUS_LED, HIGH);
    sensor_init();
    sensor_power_on();
    delay(SENSOR_SETTLE_MS);

    int   raw       = sensor_read_raw();
    float moisture  = sensor_to_percent(raw);
    float battery_v = sensor_battery_voltage();

    sensor_power_off();
    digitalWrite(PIN_STATUS_LED, LOW);

    Serial.printf("[NODE %d] moisture=%.1f%% raw=%d bat=%.2fV\n",
                  PLANT_ID, moisture, raw, battery_v);

    // ── 2. Transmit via ESP-NOW ───────────────────────────────────────────────
    SensorPacket pkt;
    pkt.plant_id     = PLANT_ID;
    pkt.moisture_pct = moisture;
    pkt.battery_v    = battery_v;
    pkt.uptime_s     = s_wake_count * (SLEEP_INTERVAL_US / 1000000ULL);

    transmitter_init();
    digitalWrite(PIN_STATUS_LED, HIGH);
    bool ok = transmitter_send(&pkt);
    digitalWrite(PIN_STATUS_LED, LOW);
    transmitter_deinit();

    delay(200);
    if (ok) {
        // 2 slow blinks = TX success
        led_blink(2, 300, 150);
    } else {
        // 5 rapid blinks = TX failed
        Serial.println("[NODE] TX failed after all retries");
        led_blink(5, 80, 80);
    }

}

void loop() {
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(1000);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(1000);
}
