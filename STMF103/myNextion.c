#include "myNextion.h"
#include "user_USART.h"
#include "user_TIMER.h"
#include "user_USART.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include <string.h>

uint8_t RX_FLAG_END_LINE = 0;
uint8_t status[21]="t3.txt=";
uint8_t command[11]="sendme";
uint32_t st=0;
uint8_t endMes[3]={0xFF,0xFF,0xFF};
static uint8_t StrBuff[64]; 

void USART_IRQProcessFunc(uint8_t RXc){
    toBuf(RXc);
    TIM_Cmd(TIM3, DISABLE);
    if (RXc == 0xFF)                                                            //Если пришел признак окончания команды (их должно быть 3)
       RX_FLAG_END_LINE++;
    if(RX_FLAG_END_LINE!=3)                                                     //Если команда пришла еще не полностью, запускаем таймер сброса
         TIM_Cmd(TIM3, ENABLE);   
}

void nextionEvent(void){
      //fromBuf(0) - номер страницы
      //fromBuf(1) - номер кнопки (элемента). 0 - переход на страницу
      //fromBuf(3) - значение 
      
      RX_FLAG_END_LINE=0;
      
      if(fromBuf(0) == 0x02 && fromBuf(1) == 0x01){                             //СТАРТ Массаж 1
        GPIO_SetBits(GPIOC,GPIO_Pin_12);
        TIM4->CCR1=fromBuf(3);
      }
      else if(fromBuf(0) == 0x02 && fromBuf(1) == 0x02){                        //ПАУЗА Массаж 1
        GPIO_ResetBits(GPIOC,GPIO_Pin_12);
        TIM4->CCR1=0;
      }
      else if(fromBuf(0) == 0x02 && fromBuf(1) == 0x03){                        //СТОП Массаж 1
        GPIO_ResetBits(GPIOC,GPIO_Pin_12);
        TIM4->CCR1=0;
      }
      else if(fromBuf(0) == 0x03 && fromBuf(1) == 0x01){                        //СТАРТ Массаж 2
        GPIO_SetBits(GPIOC,GPIO_Pin_13);
        TIM4->CCR2=fromBuf(3);
      }
      else if(fromBuf(0) == 0x03 && fromBuf(1) == 0x02){                        //ПАУЗА Массаж 2
        GPIO_ResetBits(GPIOC,GPIO_Pin_13);
        TIM4->CCR2=0;
      }
      else if(fromBuf(0) == 0x03 && fromBuf(1) == 0x03){                        //СТОП Массаж 2
        GPIO_ResetBits(GPIOC,GPIO_Pin_13);
        TIM4->CCR2=0;
      }
      else if(fromBuf(0) == 0x01 && fromBuf(1) == 0x00 &&  fromBuf(3) == 0x01){ //Запрос комплектации
          st = GPIO_ReadInputDataBit (GPIOC, GPIO_Pin_8)*2 + GPIO_ReadInputDataBit (GPIOC, GPIO_Pin_6);
          Nextion_SetValue_Number("mode.val", st);
      }
     clear_RXBuffer();
}

void Nextion_SetValue_Number(char *ValueName, uint32_t Value)
{
  sprintf((char *)StrBuff, "%s=%d", ValueName, Value);
  uint16_t Len = strlen((char *)StrBuff);
  
  USART1_put_string(StrBuff, Len);
  USART1_put_string((uint8_t *)endMes, 3);
}
void Nextion_SetValue_String(char *ValueName, char *Value)
{
  sprintf((char *)StrBuff, "%s=\"%s\"", ValueName, Value);
  uint16_t Len = strlen((char *)StrBuff);
  
  USART1_put_string(StrBuff, Len);
  USART1_put_string((uint8_t *)endMes, 3);
}

void incFLAG_END_LINE(void){
  RX_FLAG_END_LINE++;
}
void resetFLAG_END_LINE(void){
  RX_FLAG_END_LINE=0;
}
uint8_t getFLAG_END_LINE(void){
  return RX_FLAG_END_LINE;
}