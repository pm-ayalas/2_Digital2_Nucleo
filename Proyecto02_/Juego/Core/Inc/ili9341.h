#ifndef ILI9341_H
#define ILI9341_H
#include <stdint.h>
void ILI9341_Init(void);
void ILI9341_DrawRGB565(const uint16_t *pixels, uint16_t width, uint16_t height);
void ILI9341_FillScreen(uint16_t color);
uint8_t ILI9341_DrawRGB565RLE(const uint16_t *runs, uint32_t pair_count);
uint8_t ILI9341_DrawLayersRLE(const uint16_t *bg, uint32_t bg_pairs,
                              const uint16_t *fg, uint32_t fg_pairs,
                              uint16_t transparent);
void ILI9341_DrawRectRGB565(uint16_t x, uint16_t y, uint16_t w,
                          uint16_t h, const uint16_t *pixels);
#endif
