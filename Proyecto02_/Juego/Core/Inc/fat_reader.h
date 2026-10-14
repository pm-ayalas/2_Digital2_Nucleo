#ifndef FAT_READER_H
#define FAT_READER_H
#include <stdint.h>
typedef uint8_t (*FR_ReadSector)(uint32_t,uint8_t*);
uint8_t FR_Mount(FR_ReadSector read);
uint8_t FR_Open(const char name83[11]);
uint8_t FR_Read(void *dst,uint32_t count);
uint32_t FR_Size(void);
#endif
