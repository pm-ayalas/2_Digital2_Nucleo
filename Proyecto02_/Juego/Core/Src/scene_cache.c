#include "scene_cache.h"
#define PIXELS (320U * 240U)
static uint8_t pixels[PIXELS / 2U];
static uint16_t palette[16];
static uint8_t palette_size;
static uint32_t used;
static uint8_t failed;
void SceneCache_Begin(void)
{
    palette_size = 0U;
    used = 0U;
    failed = 0U;
}
void SceneCache_Append(uint16_t color, uint16_t count)
{
    if (failed) return;
    if (count > PIXELS - used) { failed = 1U; return; }
    uint8_t index = 0U;
    while (index < palette_size && palette[index] != color) index++;
    if (index == palette_size) {
        if (palette_size == 16U) { failed = 1U; return; }
        palette[palette_size++] = color;
    }
    while (count--) {
        uint32_t pos = used++;
        if (pos & 1U) pixels[pos >> 1] = (uint8_t)((pixels[pos >> 1] & 0xF0U) | index);
        else pixels[pos >> 1] = (uint8_t)(index << 4);
    }
}
uint8_t SceneCache_Ready(void)
{
    return !failed && used == PIXELS;
}
uint16_t SceneCache_Pixel(int16_t x, int16_t y)
{
    if (x < 0 || y < 0 || x >= 320 || y >= 240 || !SceneCache_Ready()) return 0U;
    uint32_t pos = (uint32_t)y * 320U + (uint16_t)x;
    uint8_t packed = pixels[pos >> 1];
    return palette[(pos & 1U) ? (packed & 15U) : (packed >> 4)];
}
