#include "main.h"
#include "ili9341.h"
#include "scene_cache.h"

#define LCD_WIDTH  320U
#define LCD_HEIGHT 240U
#define ILI9341_BLACK 0x0000U

static uint8_t ValidFullScreenRLE(const uint16_t *runs, uint32_t pairs)
{
    if (runs == 0 || pairs == 0U || pairs > LCD_WIDTH * LCD_HEIGHT) return 0U;
    uint32_t total = 0U;
    for (uint32_t i = 0U; i < pairs; i++) {
        uint16_t count = runs[2U * i];
        if (count == 0U || count > LCD_WIDTH * LCD_HEIGHT - total) return 0U;
        total += count;
    }
    return total == LCD_WIDTH * LCD_HEIGHT;
}

#define WRITE_PIN(name, high) do {                                      \
    if (high) { name##_GPIO_Port->BSRR = (uint32_t)name##_Pin; }         \
    else      { name##_GPIO_Port->BSRR = ((uint32_t)name##_Pin << 16U); } \
} while (0)

static void BusWrite(uint8_t value)
{
    WRITE_PIN(LCD_D0, (value & 0x01U) != 0U);
    WRITE_PIN(LCD_D1, (value & 0x02U) != 0U);
    WRITE_PIN(LCD_D2, (value & 0x04U) != 0U);
    WRITE_PIN(LCD_D3, (value & 0x08U) != 0U);
    WRITE_PIN(LCD_D4, (value & 0x10U) != 0U);
    WRITE_PIN(LCD_D5, (value & 0x20U) != 0U);
    WRITE_PIN(LCD_D6, (value & 0x40U) != 0U);
    WRITE_PIN(LCD_D7, (value & 0x80U) != 0U);
}

static void WriteByte(uint8_t value)
{
    BusWrite(value);
    WRITE_PIN(LCD_WR, 0U);
    __NOP(); __NOP(); __NOP(); __NOP();
    WRITE_PIN(LCD_WR, 1U);
    __NOP(); __NOP(); __NOP(); __NOP();
}

static void WriteCommand(uint8_t command, const uint8_t *data, uint16_t count)
{
    WRITE_PIN(LCD_CS, 0U);
    WRITE_PIN(LCD_RS, 0U);
    WriteByte(command);
    WRITE_PIN(LCD_RS, 1U);
    for (uint16_t i = 0U; i < count; i++) {
        WriteByte(data[i]);
    }
    WRITE_PIN(LCD_CS, 1U);
}

static void SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];
    data[0] = (uint8_t)(x0 >> 8); data[1] = (uint8_t)x0;
    data[2] = (uint8_t)(x1 >> 8); data[3] = (uint8_t)x1;
    WriteCommand(0x2AU, data, 4U);
    data[0] = (uint8_t)(y0 >> 8); data[1] = (uint8_t)y0;
    data[2] = (uint8_t)(y1 >> 8); data[3] = (uint8_t)y1;
    WriteCommand(0x2BU, data, 4U);
}

static void BeginPixels(void)
{
    WRITE_PIN(LCD_CS, 0U);
    WRITE_PIN(LCD_RS, 0U);
    WriteByte(0x2CU);
    WRITE_PIN(LCD_RS, 1U);
}

static void EndPixels(void)
{
    WRITE_PIN(LCD_CS, 1U);
}

void ILI9341_Init(void)
{
    WRITE_PIN(LCD_CS, 1U);
    WRITE_PIN(LCD_RS, 1U);
    WRITE_PIN(LCD_WR, 1U);
    WRITE_PIN(LCD_RD, 1U);
    WRITE_PIN(LCD_RST, 0U);
    HAL_Delay(20U);
    WRITE_PIN(LCD_RST, 1U);
    HAL_Delay(120U);

    WriteCommand(0x01U, 0, 0U);
    HAL_Delay(150U);
    WriteCommand(0x28U, 0, 0U);

    { const uint8_t d[] = {0x00, 0x18}; WriteCommand(0xB1U, d, sizeof d); }
    { const uint8_t d[] = {0x08, 0x82, 0x27}; WriteCommand(0xB6U, d, sizeof d); }
    { const uint8_t d[] = {0x23}; WriteCommand(0xC0U, d, sizeof d); }
    { const uint8_t d[] = {0x10}; WriteCommand(0xC1U, d, sizeof d); }
    { const uint8_t d[] = {0x3E, 0x28}; WriteCommand(0xC5U, d, sizeof d); }
    { const uint8_t d[] = {0x86}; WriteCommand(0xC7U, d, sizeof d); }
    { const uint8_t d[] = {0x28}; WriteCommand(0x36U, d, sizeof d); } /* 320x240*/
    { const uint8_t d[] = {0x55}; WriteCommand(0x3AU, d, sizeof d); }
    { const uint8_t d[] = {0x00}; WriteCommand(0xF2U, d, sizeof d); }
    { const uint8_t d[] = {0x01}; WriteCommand(0x26U, d, sizeof d); }
    { const uint8_t d[] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00}; WriteCommand(0xE0U, d, sizeof d); }
    { const uint8_t d[] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F}; WriteCommand(0xE1U, d, sizeof d); }

    WriteCommand(0x11U, 0, 0U);
    HAL_Delay(120U);
    WriteCommand(0x29U, 0, 0U);
    HAL_Delay(20U);
    SetWindow(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);
    ILI9341_FillScreen(ILI9341_BLACK);
}

void ILI9341_DrawRGB565(const uint16_t *pixels, uint16_t width, uint16_t height)
{
    if ((pixels == 0) || (width != LCD_WIDTH) || (height != LCD_HEIGHT)) return;
    SetWindow(0U, 0U, width - 1U, height - 1U);
    BeginPixels();
    for (uint32_t i = 0U; i < (uint32_t)width * height; i++) {
        uint16_t c = pixels[i];
        WriteByte((uint8_t)(c >> 8));
        WriteByte((uint8_t)c);
    }
    EndPixels();
}

void ILI9341_FillScreen(uint16_t color)
{
    SetWindow(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);
    BeginPixels();
    for (uint32_t i = 0U; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        WriteByte((uint8_t)(color >> 8));
        WriteByte((uint8_t)color);
    }
    EndPixels();
}

uint8_t ILI9341_DrawRGB565RLE(const uint16_t *runs, uint32_t pair_count)
{
    if (runs == 0 || pair_count == 0U || pair_count > LCD_WIDTH * LCD_HEIGHT)
        return 0U;
    uint32_t total = 0U;
    for (uint32_t i = 0U; i < pair_count; i++) {
        uint16_t count = runs[2U * i];
        if (count == 0U || count > LCD_WIDTH * LCD_HEIGHT - total) return 0U;
        total += count;
    }
    if (total != LCD_WIDTH * LCD_HEIGHT) return 0U;
    SetWindow(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);
    BeginPixels();
    for (uint32_t i = 0U; i < pair_count; i++) {
        uint16_t count = runs[2U * i];
        uint16_t color = runs[2U * i + 1U];
        while (count--) {
            WriteByte((uint8_t)(color >> 8));
            WriteByte((uint8_t)color);
        }
    }
    EndPixels();
    return 1U;
}

uint8_t ILI9341_DrawLayersRLE(const uint16_t *bg, uint32_t bg_pairs,
                              const uint16_t *fg, uint32_t fg_pairs,
                              uint16_t transparent)
{
    if (!ValidFullScreenRLE(bg, bg_pairs) || !ValidFullScreenRLE(fg, fg_pairs))
        return 0U;

    SceneCache_Begin();
    uint32_t bi = 0U, fi = 0U;
    uint16_t bn = bg[0], fn = fg[0];
    uint32_t remaining = LCD_WIDTH * LCD_HEIGHT;
    SetWindow(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);
    BeginPixels();
    while (remaining != 0U) {
        uint16_t count = bn < fn ? bn : fn;
        uint16_t foreground = fg[2U * fi + 1U];
        uint16_t color = foreground == transparent ? bg[2U * bi + 1U] : foreground;
        SceneCache_Append(color, count);
        for (uint16_t j = 0U; j < count; j++) {
            WriteByte((uint8_t)(color >> 8));
            WriteByte((uint8_t)color);
        }
        remaining -= count;
        bn -= count;
        fn -= count;
        if (remaining != 0U) {
            if (bn == 0U) bn = bg[2U * ++bi];
            if (fn == 0U) fn = fg[2U * ++fi];
        }
    }
    EndPixels();
    return SceneCache_Ready();
}

void ILI9341_DrawRectRGB565(uint16_t x, uint16_t y, uint16_t w,
                          uint16_t h, const uint16_t *pixels)
{
    if (!pixels || !w || !h || x >= LCD_WIDTH || y >= LCD_HEIGHT ||
        w > LCD_WIDTH - x || h > LCD_HEIGHT - y) return;
    SetWindow(x, y, x + w - 1U, y + h - 1U);
    BeginPixels();
    for (uint32_t i = 0U; i < (uint32_t)w * h; i++) {
        WriteByte((uint8_t)(pixels[i] >> 8));
        WriteByte((uint8_t)pixels[i]);
    }
    EndPixels();
}
