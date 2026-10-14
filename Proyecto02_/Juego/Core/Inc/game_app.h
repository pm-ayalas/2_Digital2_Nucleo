#ifndef GAME_APP_H
#define GAME_APP_H
#include <stdint.h>
void GameApp_Init(void);

uint8_t GameApp_Poll(void);

uint8_t GameApp_GetSelectedLevel(void);
#endif
