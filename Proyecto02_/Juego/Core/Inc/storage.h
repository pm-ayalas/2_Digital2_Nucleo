#ifndef STORAGE_H
#define STORAGE_H
#include <stdint.h>
void Storage_Init(void);
uint8_t Storage_Draw(const char name83[11],uint8_t cache);
extern volatile uint32_t g_sd_status,g_sd_loaded_frames;

#endif
