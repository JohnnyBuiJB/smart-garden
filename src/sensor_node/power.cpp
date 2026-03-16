#include "power.h"
#include <Arduino.h>
#include <esp_sleep.h>

void power_deep_sleep(uint64_t sleep_us) {
    esp_sleep_enable_timer_wakeup(sleep_us);
    Serial.flush();
    esp_deep_sleep_start();
}
