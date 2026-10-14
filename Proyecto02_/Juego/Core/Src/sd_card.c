
#include "main.h"
#include "sd_card.h"
static uint8_t high_capacity,io_fault;
static uint8_t X(uint8_t b){
 uint32_t t=HAL_GetTick();if(io_fault)return 255;
 while(!(SPI1->SR&SPI_SR_TXE))if(HAL_GetTick()-t>20){io_fault=1;return 255;}
 *(__IO uint8_t *)&SPI1->DR=b;
 while(!(SPI1->SR&SPI_SR_RXNE))if(HAL_GetTick()-t>20){io_fault=1;return 255;}
 return *(__IO uint8_t *)&SPI1->DR;
}
static void Deselect(void){HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);X(255);}
static uint8_t Ready(uint32_t ms){uint32_t t=HAL_GetTick();do{if(X(255)==255&&!io_fault)return 1;}while(!io_fault&&HAL_GetTick()-t<ms);return 0;}
static uint8_t Command(uint8_t c,uint32_t arg){
 Deselect();HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
 if(!Ready(100))return 255;
 X(0x40|c);X(arg>>24);X(arg>>16);X(arg>>8);X(arg);
 X(c==0?0x95:c==8?0x87:1);
 for(unsigned i=0;i<16;i++){uint8_t r=X(255);if(!(r&128))return r;}return 255;
}
uint8_t SDCard_Init(void){
 io_fault=0;high_capacity=0;__HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_SPI1_CLK_ENABLE();
 GPIO_InitTypeDef g={0};g.Pin=GPIO_PIN_6;g.Mode=GPIO_MODE_OUTPUT_PP;g.Speed=GPIO_SPEED_FREQ_HIGH;HAL_GPIO_Init(GPIOB,&g);
 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
 g.Pin=GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;g.Mode=GPIO_MODE_AF_PP;g.Alternate=GPIO_AF5_SPI1;g.Pull=GPIO_PULLUP;HAL_GPIO_Init(GPIOA,&g);
 SPI1->CR1=0;SPI1->CR2=0;SPI1->CR1=SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI|SPI_CR1_BR|SPI_CR1_SPE;
 HAL_Delay(10);for(unsigned i=0;i<10;i++)X(255); /* >=74 clocks con CS alto. */
 uint32_t t=HAL_GetTick();uint8_t r;
 do{r=Command(0,0);Deselect();}while(r!=1&&!io_fault&&HAL_GetTick()-t<1000);
 if(r!=1)return 0;
 r=Command(8,0x1AA);uint8_t v2=r==1;
 if(v2){uint8_t a=X(255),b=X(255),c=X(255),d=X(255);(void)a;(void)b;if(c!=1||d!=0xAA){Deselect();return 0;}}
 else if(r!=5){Deselect();return 0;}
 Deselect();t=HAL_GetTick();
 do{r=Command(55,0);Deselect();if(r>1)return 0;r=Command(41,v2?0x40000000:0);Deselect();}while(r==1&&!io_fault&&HAL_GetTick()-t<2000);
 if(r!=0)return 0;
 r=Command(58,0);if(r){Deselect();return 0;}
 uint8_t ocr=X(255);X(255);X(255);X(255);Deselect();high_capacity=v2&&((ocr&0x40)!=0);
 if(!high_capacity){r=Command(16,512);Deselect();if(r)return 0;}
 SPI1->CR1&=~SPI_CR1_SPE;
 SPI1->CR1=(SPI1->CR1&~SPI_CR1_BR)|SPI_CR1_BR_2|SPI_CR1_SPE;
 return !io_fault;
}
uint8_t SDCard_Read(uint32_t lba,uint8_t data[512]){
 if(io_fault||(!high_capacity&&lba>0x7FFFFFu))return 0;
 if(Command(17,high_capacity?lba:lba*512u)!=0){Deselect();return 0;}
 uint32_t t=HAL_GetTick();uint8_t token;
 do{token=X(255);}while(token==255&&!io_fault&&HAL_GetTick()-t<200);
 if(token!=0xFE){Deselect();return 0;}
 for(unsigned i=0;i<512;i++)data[i]=X(255);
 X(255);X(255);Deselect();return !io_fault;
}
