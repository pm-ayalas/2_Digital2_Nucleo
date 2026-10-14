#ifndef GAMEPLAY_H
#define GAMEPLAY_H
#include <stdint.h>
#define PLAY_RED_LEFT   (1U << 0)
#define PLAY_RED_RIGHT  (1U << 1)
#define PLAY_RED_JUMP   (1U << 2)
#define PLAY_BLUE_LEFT  (1U << 3)
#define PLAY_BLUE_RIGHT (1U << 4)
#define PLAY_BLUE_JUMP  (1U << 5)
#define PLAY_PAUSE      (1U << 6)

uint8_t Gameplay_Start(uint8_t level);
uint8_t Gameplay_Update(uint8_t raw_buttons);
uint8_t Gameplay_IsWon(void);
// reinicio nivel
uint8_t Gameplay_Resume(void);
#endif
