#ifndef BOARD_LINK_H
#define BOARD_LINK_H
#include <stdint.h>
void BoardLink_Init(void);
uint8_t BoardLink_Buttons(void);
uint8_t BoardLink_Connected(void);
void BoardLink_AudioState(uint8_t level);
void BoardLink_Effect(uint8_t effect);
// 1=salto
// 2=muerte/reinicio conjunto
// 3=victoria. */
extern volatile uint32_t g_link_frames,g_link_errors;
extern volatile uint32_t g_rx_bytes[2],g_rx_bad_crc[2];
extern volatile uint8_t g_rx_type[2];
extern volatile uint32_t g_fuego_frames,g_agua_frames;
#endif
