#include "storage.h"
#include "sd_card.h"
#include "fat_reader.h"
#include "ili9341.h"
#include "scene_cache.h"
#include <string.h>
volatile uint32_t g_sd_status,g_sd_loaded_frames;
static uint8_t mounted;
static uint16_t row[320];
static uint32_t U32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
void Storage_Init(void){
 mounted=0;if(!SDCard_Init()){g_sd_status=2;return;}
 if(!FR_Mount(SDCard_Read)){g_sd_status=3;return;}mounted=1;g_sd_status=1;
}
uint8_t Storage_Draw(const char name[11],uint8_t cache){
 if(!mounted)return 0;
 if(!FR_Open(name)){g_sd_status=4;return 0;}
 uint8_t h[16];if(FR_Size()!=153616||!FR_Read(h,16)||memcmp(h,"FAI1",4)||U32(h+4)!=320||U32(h+8)!=240){g_sd_status=5;return 0;}
 uint32_t crc=0xFFFFFFFF,expected=U32(h+12);
 if(cache)SceneCache_Begin();
 for(unsigned y=0;y<240;y++){
  if(!FR_Read(row,640)){g_sd_status=6;return 0;}
  uint8_t *b=(uint8_t*)row;
  for(unsigned i=0;i<640;i++){crc^=b[i];for(unsigned k=0;k<8;k++)crc=(crc>>1)^((crc&1)?0xEDB88320u:0);}
  if(cache)for(unsigned x=0;x<320;x++)SceneCache_Append(row[x],1);
  ILI9341_DrawRectRGB565(0,y,320,1,row);
 }
 if((crc^0xFFFFFFFFu)!=expected){g_sd_status=7;return 0;}
 if(cache&&!SceneCache_Ready()){g_sd_status=8;return 0;}
 g_sd_status=1;g_sd_loaded_frames++;return 1;
}
