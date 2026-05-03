#pragma once
#include <stdint.h>

void storage_init();

// Records when a plant was last watered (Unix-style seconds counter).
void     storage_save_last_watered(uint8_t plant_id, uint32_t timestamp_s);
uint32_t storage_get_last_watered(uint8_t plant_id);

// Tracks consecutive cycles where no reading arrived from a node (for diagnostics).
void    storage_inc_missed(uint8_t plant_id);
void    storage_clear_missed(uint8_t plant_id);
uint8_t storage_get_missed(uint8_t plant_id);
