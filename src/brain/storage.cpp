#include "storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences s_prefs;

void storage_init() {
    s_prefs.begin("sg_brain", false);
}

void storage_save_last_watered(uint8_t plant_id, uint32_t timestamp_s) {
    char key[16];
    snprintf(key, sizeof(key), "lw%d", plant_id);
    s_prefs.putUInt(key, timestamp_s);
}

uint32_t storage_get_last_watered(uint8_t plant_id) {
    char key[16];
    snprintf(key, sizeof(key), "lw%d", plant_id);
    return s_prefs.getUInt(key, 0);
}

void storage_inc_missed(uint8_t plant_id) {
    char key[16];
    snprintf(key, sizeof(key), "ms%d", plant_id);
    uint8_t val = s_prefs.getUChar(key, 0);
    s_prefs.putUChar(key, val + 1);
}

void storage_clear_missed(uint8_t plant_id) {
    char key[16];
    snprintf(key, sizeof(key), "ms%d", plant_id);
    s_prefs.putUChar(key, 0);
}

uint8_t storage_get_missed(uint8_t plant_id) {
    char key[16];
    snprintf(key, sizeof(key), "ms%d", plant_id);
    return s_prefs.getUChar(key, 0);
}
