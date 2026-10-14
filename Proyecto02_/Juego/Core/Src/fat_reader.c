#include "fat_reader.h"
#include <string.h>
static FR_ReadSector read_sector;
static uint8_t buf[512],spc;
static uint32_t fat,data,root,clusters,total_file,remaining,file_cluster,file_sector,offset;
static uint16_t U16(const uint8_t *p){return (uint16_t)(p[0]|p[1]<<8);}
static uint32_t U32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint8_t ValidCluster(uint32_t c){return c>=2&&c<clusters+2;}
static uint32_t Next(uint32_t c){
 if(!ValidCluster(c)||!read_sector(fat+c/128,buf))return 0;
 return U32(buf+(c%128)*4)&0x0FFFFFFF;
}
static uint8_t Boot(uint32_t start){
 if(!read_sector(start,buf)||U16(buf+510)!=0xAA55||U16(buf+11)!=512||U16(buf+17)!=0||U16(buf+22)!=0)return 0;
 spc=buf[13];uint32_t reserved=U16(buf+14),fats=buf[16],fs=U32(buf+36),total=U32(buf+32);uint16_t flags=U16(buf+40);
 if(!spc||spc>128||(spc&(spc-1))||!reserved||!fats||fats>2||!fs||fs>0xFFFFFFFFu/fats)return 0;
 uint32_t overhead=reserved+fats*fs;
 if(overhead<reserved||total<=overhead||start>0xFFFFFFFFu-total)return 0;
 clusters=(total-overhead)/spc;if(clusters<65525||clusters>0x0FFFFFEDu||fs<(clusters+2+127)/128)return 0;
 fat=start+reserved;
 if(flags&0x80){unsigned active=flags&15;if(active>=fats)return 0;fat+=active*fs;}
 data=start+overhead;root=U32(buf+44)&0x0FFFFFFF;return ValidCluster(root);
}
uint8_t FR_Mount(FR_ReadSector reader){
 read_sector=reader;remaining=total_file=0;
 if(Boot(0))return 1;
 if(!read_sector(0,buf)||U16(buf+510)!=0xAA55)return 0;
 uint32_t starts[4];for(unsigned i=0;i<4;i++)starts[i]=((buf[450+i*16]==0x0B)||(buf[450+i*16]==0x0C))?U32(buf+454+i*16):0;
 for(unsigned i=0;i<4;i++)if(starts[i]&&Boot(starts[i]))return 1;
 return 0;
}
uint8_t FR_Open(const char name[11]){
 uint32_t c=root;remaining=total_file=0;
 for(uint32_t visited=0;visited<clusters&&ValidCluster(c);visited++){
  for(unsigned s=0;s<spc;s++){
   if(!read_sector(data+(c-2)*spc+s,buf))return 0;
   for(unsigned o=0;o<512;o+=32){
    if(buf[o]==0)return 0;
    if(buf[o]==0xE5||(buf[o+11]&0x18)||buf[o+11]==0x0F)continue;
    if(!memcmp(buf+o,name,11)){
     total_file=remaining=U32(buf+o+28);file_cluster=((uint32_t)U16(buf+o+20)<<16)|U16(buf+o+26);
     file_sector=offset=0;return remaining==0||ValidCluster(file_cluster);
    }
   }
  }
  c=Next(c);
 }
 return 0;
}
uint32_t FR_Size(void){return total_file;}
uint8_t FR_Read(void *dst,uint32_t n){
 uint8_t *out=dst;if(n>remaining)return 0;
 while(n){
  if(!ValidCluster(file_cluster)||!read_sector(data+(file_cluster-2)*spc+file_sector,buf))return 0;
  unsigned take=512-offset;if(take>n)take=n;memcpy(out,buf+offset,take);
  out+=take;n-=take;remaining-=take;offset+=take;
  if(offset==512){offset=0;if(++file_sector==spc){file_sector=0;if(remaining){file_cluster=Next(file_cluster);if(!ValidCluster(file_cluster))return 0;}}}
 }
 return 1;
}
