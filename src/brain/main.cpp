#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "irrigation.h"

static WiFiClient   s_wifi;
static PubSubClient s_mqtt(s_wifi);

// Topic format: garden/plants/{id}/irrigate
// Payload format: {"duration_ms": 5000}
static void on_mqtt_message(const char *topic, byte *payload, unsigned int len) {
    int plant_id = -1;
    sscanf(topic, "garden/plants/%d/irrigate", &plant_id);
    if (plant_id < 0 || plant_id >= NUM_PLANTS) return;

    char buf[64];
    memcpy(buf, payload, min(len, (unsigned int)sizeof(buf) - 1));
    buf[min(len, (unsigned int)sizeof(buf) - 1)] = '\0';

    uint32_t duration_ms = 0;
    sscanf(buf, "{\"duration_ms\":%lu}", &duration_ms);
    if (duration_ms == 0) return;

    Serial.printf("[BRAIN] plant=%d duration=%lums\n", plant_id, (unsigned long)duration_ms);
    irrigation_water_plant((uint8_t)plant_id, duration_ms);
}

static void connect_wifi() {
    Serial.printf("[BRAIN] connecting to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[BRAIN] IP %s\n", WiFi.localIP().toString().c_str());
}

static void connect_mqtt() {
    while (!s_mqtt.connected()) {
        Serial.println("[BRAIN] connecting to MQTT broker...");
        if (s_mqtt.connect("irrigation_controller")) {
            s_mqtt.subscribe("garden/plants/+/irrigate");
            Serial.println("[BRAIN] MQTT connected");
        } else {
            Serial.printf("[BRAIN] MQTT failed rc=%d, retry in 2s\n", s_mqtt.state());
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    irrigation_init();
    connect_wifi();
    s_mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    s_mqtt.setCallback(on_mqtt_message);
    connect_mqtt();
}

void loop() {
    if (!s_mqtt.connected()) connect_mqtt();
    s_mqtt.loop();
}
