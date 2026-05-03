#include "transmitter.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

static volatile bool s_ack_received = false;
static volatile bool s_ack_success  = false;

static uint8_t s_brain_mac[] = BRAIN_MAC;

static void on_data_sent(const uint8_t *mac, esp_now_send_status_t status) {
    s_ack_success  = (status == ESP_NOW_SEND_SUCCESS);
    s_ack_received = true;
}

void transmitter_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[TX] esp_now_init failed");
        return;
    }

    esp_now_register_send_cb(on_data_sent);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, s_brain_mac, 6);
    peer.channel = 0;   // use current channel
    peer.encrypt = false;
    esp_now_add_peer(&peer);
}

bool transmitter_send(const SensorPacket *pkt) {
    for (int attempt = 0; attempt < ESPNOW_RETRIES; attempt++) {
        s_ack_received = false;
        s_ack_success  = false;

        esp_err_t err = esp_now_send(
            s_brain_mac,
            (const uint8_t *)pkt,
            sizeof(SensorPacket)
        );

        if (err != ESP_OK) {
            Serial.printf("[TX] send error %d, attempt %d\n", err, attempt + 1);
            delay(200);
            continue;
        }

        unsigned long start = millis();
        while (!s_ack_received && (millis() - start) < ESPNOW_TIMEOUT_MS) {
            delay(10);
        }

        if (s_ack_success) {
            Serial.printf("[TX] ACK ok (attempt %d)\n", attempt + 1);
            return true;
        }

        Serial.printf("[TX] no ACK, attempt %d\n", attempt + 1);
        delay(500);
    }
    return false;
}

void transmitter_deinit() {
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
}
