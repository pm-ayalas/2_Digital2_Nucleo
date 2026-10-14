#ifndef SCENE_CACHE_H
#define SCENE_CACHE_H
#include <stdint.h>
void SceneCache_Begin(void);
void SceneCache_Append(uint16_t color, uint16_t count);
uint8_t SceneCache_Ready(void);
uint16_t SceneCache_Pixel(int16_t x, int16_t y);
#endif
