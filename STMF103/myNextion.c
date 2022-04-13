#include "myNextion.h"
#include "user_USART.h"
#include "user_TIMER.h"
#include "user_USART.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include <string.h>
#include "LED_user.h"
#include "i2c_ee.h"
#include "main.h"

uint8_t RX_FLAG_END_LINE = 0;
uint8_t status[21]="t3.txt=";
uint8_t command[11]="sendme";
//uint32_t st=0;
uint8_t autoBright = 1;
uint8_t endMes[3]={0xFF,0xFF,0xFF};
static uint8_t StrBuff[64]; 
uint8_t nowPage=0;
uint8_t reqBigBuf=0;

uint16_t graphDataBuf[600]; 
extern uint16_t hourT1Buf[HOURBUFSIZE];  
extern uint16_t hourT2Buf[HOURBUFSIZE];  
extern uint16_t hourHBuf[HOURBUFSIZE];  
extern uint16_t hourPBuf[HOURBUFSIZE];  

uint8_t commands[3][8];
uint8_t wrCommand=0,rdCommand=0, countCommand=0;


void nextionEvent(void){
      //fromBuf(0) - номер страницы
      //fromBuf(1) - номер кнопки (элемента). 0 - переход на страницу
      //fromBuf(3) - значение 
      countCommand--;
      uint8_t page,element,value;
      page = commands[rdCommand][0];
      element = commands[rdCommand][1];
      value = commands[rdCommand][3];
      rdCommand++;
      if(rdCommand ==3) rdCommand=0;
      
      if(page==0xFF || element ==0xFF) {                                        //Экран отправляет сигнал об ошибке
        errorProcess(page);
        return;
      }
      
      nowPage = page;
      if(page == 0x01 || page == 0x02 || page == 0x03 || page == 0x04){                            
        if(element != 0x00){
          readGrapgData((page-1)*4 + (element-1),graphDataBuf);        //Читаем данные с EEPROM (600 точек) с параметром номера страницы минус 1.
          transparent(graphDataBuf);
          for(int i=0;i<600;i++){
            Nextion_SetValue_Number("add 2,0,",(uint32_t)graphDataBuf[i]); 
          }
        }
      }
      else if(page == 0x05){                                                  //Экран настроек)
        autoBright = value;
      }
      else{
        if(element == 0x00){
            sensorsDataSend();
          }
          if(element == 0x01){
            GPIOB->ODR ^= GPIO_Pin_0;
          }
          else if(element == 0x02){
            GPIOB->ODR ^= GPIO_Pin_5;
          }
          else if(element == 0x03){
            GPIOB->ODR ^= GPIO_Pin_8;
          }
          else if(element == 0x04){
            GPIOB->ODR ^= GPIO_Pin_9;
          }
     }
}

void Nextion_SetValue_Number(char *ValueName, int32_t Value)
{
  sprintf((char *)StrBuff, "%s%d", ValueName, Value);
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

uint8_t getAutoBr(){
  return autoBright;
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

uint8_t getNowPage(void){
  return nowPage;
}
void readGrapgData(int16_t par, uint16_t *buf){
  uint16_t pageCounter;

  if(par<0) return;
  
  
  else if(par==0)                                                             //Каждый четрвертый параметр - это данные за час
    memcpy(buf,hourT1Buf,1200);
  else if(par==4)                                                             //Каждый четрвертый параметр - это данные за час
    memcpy(buf,hourT2Buf,1200);
  else if(par==8)                                                             //Каждый четрвертый параметр - это данные за час
    memcpy(buf,hourHBuf,1200);
  else if(par==12)                                                             //Каждый четрвертый параметр - это данные за час
    memcpy(buf,hourPBuf,1200);
  
  else{
      if(getFromEE(par*10,60,&pageCounter)!=0) while(1);           
      uint16_t startAddr = par*10+pageCounter;
      for(int i= 0;i<10;i++){
        readPageEE(buf,startAddr++);
        if(startAddr==(par+1)*10) startAddr = par*10;
        buf+=60;
      }
      
  }
}
void transparent(uint16_t *buf){
  int16_t max = 0;
  int16_t min = 0x7FFF;
  int32_t average=0;
  for(int i=0;i<600;i++){
    if(buf[i] > max) max = buf[i];
    if(buf[i] < min) min = buf[i];
  }
  Nextion_SetValue_Number("x1.val=",(int32_t)max);
  Nextion_SetValue_Number("x2.val=",(int32_t)min);
  min-=10;
  for(int i=0;i<600;i++){
    average+=graphDataBuf[i];
    buf[i] = (uint16_t)(255.0/(max-min)*(buf[i]-min));
    
  }
  Nextion_SetValue_Number("x0.val=",(int32_t)(average/600.0));
}
void errorProcess(uint8_t code){

}

void addCommand(){
    memcpy(commands[wrCommand++],rxBuf(),8);
    if(wrCommand==3) wrCommand=0;
    countCommand++;
    if(countCommand==1 && getTXcounter()==0)                                        //Если очередь пустая и отправлены все предыдущие байты, прямо из прерывания вызываем обработку команды
      nextionEvent();
    clear_RXBuffer();
}
uint8_t getCountCommand(){
    return countCommand;
}