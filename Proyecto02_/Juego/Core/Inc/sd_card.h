#ifndef SD_CARD_H
#define SD_CARD_H
#include <stdint.h>
uint8_t SDCard_Init(void);
uint8_t SDCard_Read(uint32_t lba,uint8_t data[512]);
#endif
