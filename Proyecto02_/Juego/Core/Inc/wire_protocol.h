#ifndef WIRE_PROTOCOL_H
#define WIRE_PROTOCOL_H
#include <stdint.h>
#include <string.h>

static inline uint8_t WireCRC(const uint8_t *p,unsigned n){
 uint8_t c=0;while(n--){c^=*p++;for(unsigned i=0;i<8;i++)c=(uint8_t)((c<<1)^((c&128)?7:0));}return c;
}
static inline void WireMake(uint8_t p[8],uint8_t type,uint8_t a,uint8_t b,uint8_t seq){
 p[0]=0xA5;p[1]=0x5A;p[2]=1;p[3]=type;p[4]=a;p[5]=b;p[6]=seq;p[7]=WireCRC(p+2,5);
}
typedef struct{uint8_t p[8],used;} WireParser;
static inline int WireFeed(WireParser *s,uint8_t b){
 if(s->used==8){memmove(s->p,s->p+1,7);s->used=7;}s->p[s->used++]=b;
 return s->used==8 && s->p[0]==0xA5 && s->p[1]==0x5A && s->p[2]==1 && s->p[7]==WireCRC(s->p+2,5);
}
#endif
