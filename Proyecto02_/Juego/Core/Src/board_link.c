#include "main.h"
#include "board_link.h"
#include "dual_input.h"
// Fuego: USART3 RX PC11 CN7-2. Agua: UART5 RX PD2 CN7-4.
static PlayerInput players[2];
static uint8_t delivered;
static uint32_t delivery_at[8];
static uint8_t sequence,last_level=255;
static uint32_t heartbeat;
volatile uint32_t g_link_frames,g_link_errors;
volatile uint32_t g_rx_bytes[2],g_rx_bad_crc[2];
volatile uint8_t g_rx_type[2];
volatile uint32_t g_fuego_frames,g_agua_frames;
volatile uint8_t g_connected_mask,g_buttons;
void BoardLink_Init(void){
 __HAL_RCC_GPIOC_CLK_ENABLE();__HAL_RCC_GPIOD_CLK_ENABLE();
 __HAL_RCC_USART3_CLK_ENABLE();__HAL_RCC_UART5_CLK_ENABLE();
 GPIO_InitTypeDef g={0};g.Pin=GPIO_PIN_10|GPIO_PIN_11;g.Mode=GPIO_MODE_AF_PP;
 g.Pull=GPIO_PULLUP;g.Speed=GPIO_SPEED_FREQ_VERY_HIGH;g.Alternate=GPIO_AF7_USART3;HAL_GPIO_Init(GPIOC,&g);
 g.Pin=GPIO_PIN_2;g.Alternate=GPIO_AF8_UART5;HAL_GPIO_Init(GPIOD,&g);
 USART3->CR1=0;USART3->CR2=0;USART3->CR3=0;
 USART3->BRR=(HAL_RCC_GetPCLK1Freq()+57600u)/115200u;
 USART3->CR1=USART_CR1_UE|USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE;
 UART5->CR1=0;UART5->CR2=0;UART5->CR3=0;
 UART5->BRR=(HAL_RCC_GetPCLK1Freq()+57600u)/115200u;
 UART5->CR1=USART_CR1_UE|USART_CR1_RE|USART_CR1_RXNEIE;
 HAL_NVIC_SetPriority(USART3_IRQn,2,0);HAL_NVIC_EnableIRQ(USART3_IRQn);
 HAL_NVIC_SetPriority(UART5_IRQn,2,0);HAL_NVIC_EnableIRQ(UART5_IRQn);
}
static void Receive(USART_TypeDef *uart,unsigned i){
 uint32_t s=uart->SR;
 if(s&(USART_SR_RXNE|USART_SR_ORE|USART_SR_FE|USART_SR_NE)){
  uint8_t b=(uint8_t)uart->DR;g_rx_bytes[i]++;
  if(s&(USART_SR_ORE|USART_SR_FE|USART_SR_NE)){
   players[i].parser.used=0;g_link_errors++;return;
  }
  uint32_t before=players[i].errors;
  PlayerInput_Byte(&players[i],i,b,HAL_GetTick());
  g_link_errors+=players[i].errors-before;
  WireParser *p=&players[i].parser;
  if(p->used==8 && p->p[0]==0xA5 && p->p[1]==0x5A && p->p[2]==1){
   if(p->p[7]==WireCRC(p->p+2,5))g_rx_type[i]=p->p[3];
   else g_rx_bad_crc[i]++;
  }
  g_fuego_frames=players[0].frames;g_agua_frames=players[1].frames;
  g_link_frames=g_fuego_frames+g_agua_frames;
 }
}
void USART3_IRQHandler(void){Receive(USART3,0);}
void UART5_IRQHandler(void){Receive(UART5,1);}
static uint8_t ConnectedAt(uint32_t now){
 return (PlayerInput_Alive(&players[0],now)?1u:0u) |
        (PlayerInput_Alive(&players[1],now)?2u:0u);
}
uint8_t BoardLink_Connected(void){
 uint32_t primask=__get_PRIMASK();__disable_irq();
 uint8_t c=ConnectedAt(HAL_GetTick());g_connected_mask=c;
 __set_PRIMASK(primask);return c;
}
uint8_t BoardLink_Buttons(void){
 uint32_t primask=__get_PRIMASK();__disable_irq();
 uint32_t now=HAL_GetTick();uint8_t c=ConnectedAt(now);g_connected_mask=c;
 for(unsigned i=0;i<2;i++)if(!(c&(1u<<i))){players[i].buttons=0;players[i].pending=0;}
 uint8_t pending=players[0].pending|(players[1].pending<<4);
 delivered&=pending;
 for(unsigned i=0;i<8;i++){
  uint8_t bit=(uint8_t)(1u<<i);
  if((pending&bit)&&!(delivered&bit)){delivery_at[i]=now;delivered|=bit;}
  else if((delivered&bit)&&now-delivery_at[i]>=60u){pending&=(uint8_t)~bit;delivered&=(uint8_t)~bit;}
 }
 players[0].pending=pending&15u;players[1].pending=pending>>4;
 uint8_t result=players[0].buttons|(players[1].buttons<<4)|pending;
 g_buttons=result;__set_PRIMASK(primask);return result;
}
static void Send(uint8_t type,uint8_t a){
 uint8_t p[8];WireMake(p,type,a,0,sequence++);
 for(unsigned i=0;i<8;i++){
  uint32_t t=HAL_GetTick();while(!(USART3->SR&USART_SR_TXE))if(HAL_GetTick()-t>5)return;
  USART3->DR=p[i];
 }
}
void BoardLink_AudioState(uint8_t level){
 uint32_t now=HAL_GetTick();
 if(level!=last_level || now-heartbeat>=100u){Send(0x20,level);last_level=level;heartbeat=now;}
}
void BoardLink_Effect(uint8_t effect){Send(0x21,effect);}
