#include "power.h"
#include "config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_adc_cal.h>

static esp_adc_cal_characteristics_t adc_chars;

void power_init() {
    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        0,
        &adc_chars
    );
    analogSetAttenuation(ADC_11db);
}

float power_battery_voltage() {
    long sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN_BATTERY_ADC);
        delay(2);
    }
    uint32_t mv = esp_adc_cal_raw_to_voltage((int)(sum / 10), &adc_chars);
    return (float)mv / 1000.0f * BATTERY_DIVIDER_RATIO;
}

bool power_is_low_battery() {
    return power_battery_voltage() < BATTERY_LOW_VOLTS;
}

void power_deep_sleep(uint64_t sleep_us) {
    esp_sleep_enable_timer_wakeup(sleep_us);
    Serial.flush();
    esp_deep_sleep_start();
}
