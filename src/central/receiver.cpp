#include "receiver.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Survives deep sleep so readings persist across the receive window even if
// the central briefly loses power (edge case; NVS handles long-term storage).
static RTC_DATA_ATTR SensorPacket s_packets[NUM_PLANTS];
static RTC_DATA_ATTR bool         s_received[NUM_PLANTS];

static void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len != sizeof(SensorPacket)) return;

    SensorPacket pkt;
    memcpy(&pkt, data, sizeof(SensorPacket));

    if (pkt.plant_id >= NUM_PLANTS) return;

    s_packets[pkt.plant_id]  = pkt;
    s_received[pkt.plant_id] = true;

    Serial.printf("[RX] plant=%d moisture=%.1f%% bat=%.2fV\n",
                  pkt.plant_id, pkt.moisture_pct, pkt.battery_v);
}

void receiver_init() {
    // Clear stale flags from previous cycle
    for (int i = 0; i < NUM_PLANTS; i++) {
        s_received[i] = false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[RX] esp_now_init failed");
        return;
    }
    esp_now_register_recv_cb(on_data_recv);
    Serial.println("[RX] listening...");
}

int receiver_collect(uint32_t timeout_ms) {
    unsigned long start = millis();
    while ((millis() - start) < timeout_ms) {
        delay(100);
    }

    int count = 0;
    for (int i = 0; i < NUM_PLANTS; i++) {
        if (s_received[i]) count++;
    }
    Serial.printf("[RX] received from %d/%d nodes\n", count, NUM_PLANTS);
    return count;
}

bool receiver_get(uint8_t plant_id, SensorPacket *out) {
    if (plant_id >= NUM_PLANTS || !s_received[plant_id]) return false;
    *out = s_packets[plant_id];
    return true;
}

void receiver_deinit() {
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
}
