#ifndef PAUSE_SCREEN_H
#define PAUSE_SCREEN_H
#include <stdint.h>
#define PAUSE_CHOICE_MENU 0U
#define PAUSE_CHOICE_CONTINUE 1U
void Pause_Draw(void);
void Pause_DrawChoice(uint8_t choice);
#endif
