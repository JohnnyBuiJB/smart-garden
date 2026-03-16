#include "sensor.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>
#include <esp_adc_cal.h>

static esp_adc_cal_characteristics_t adc_chars;

void sensor_init() {
    pinMode(PIN_SENSOR_POWER, OUTPUT);
    sensor_power_off();  // default off

    // Characterise ADC for accurate millivolt readings (uses eFuse Vref if burned)
    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        0,
        &adc_chars
    );
    analogSetAttenuation(ADC_11db);
}

void sensor_power_on() {
    digitalWrite(PIN_SENSOR_POWER, LOW);   // P-FET: LOW = sensor powered
}

void sensor_power_off() {
    digitalWrite(PIN_SENSOR_POWER, HIGH);  // P-FET: HIGH = sensor off
}

int sensor_read_raw() {
    long sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(PIN_SENSOR_ADC);
        delay(5);
    }
    return (int)(sum / ADC_SAMPLES);
}

float sensor_to_percent(int raw) {
    Preferences prefs;
    int dry = SENSOR_DRY_RAW;
    int wet = SENSOR_WET_RAW;

    prefs.begin("sg_cal", true);  // read-only namespace
    dry = prefs.getInt("dry", dry);
    wet = prefs.getInt("wet", wet);
    prefs.end();

    // Higher raw value = drier soil (capacitive sensors invert the output)
    float pct = (float)(dry - raw) / (float)(dry - wet) * 100.0f;
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

float sensor_battery_voltage() {
    long sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(PIN_BATTERY_ADC);
        delay(2);
    }
    int raw = (int)(sum / ADC_SAMPLES);

    uint32_t mv = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    return (float)mv / 1000.0f * BATTERY_DIVIDER_RATIO;
}
