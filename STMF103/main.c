#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "user_USART.h"
#include "user_GPIO.h"
#include "user_TIMER.h"
#include "user_ADC.h"
#include "myNextion.h"
#include "user_ds18b20.h"
#include "LED_user.h"
#include "i2c_ee.h"
#include "bme68x.h"
#include "userBME680.h"
#include "main.h"


int errorSensor =0;
uint16_t temper;

uint8_t isEmpty=1;
uint16_t temper=0;
uint16_t tempBuf[512];
uint16_t hourT1Buf[HOURBUFSIZE], hourT2Buf[HOURBUFSIZE],  hourHBuf[HOURBUFSIZE],  hourPBuf[HOURBUFSIZE];      //Хранит текущие отсчеты (1р/6 сек) за час
uint16_t dayT1Buf[ANOTHERBUFSIZE], dayT2Buf[ANOTHERBUFSIZE], dayHBuf[ANOTHERBUFSIZE], dayPBuf[ANOTHERBUFSIZE];                                                
uint16_t weekT1Buf[ANOTHERBUFSIZE], weekT2Buf[ANOTHERBUFSIZE], weekHBuf[ANOTHERBUFSIZE], weekPBuf[ANOTHERBUFSIZE];
uint16_t monthT1Buf[ANOTHERBUFSIZE], monthT2Buf[ANOTHERBUFSIZE], monthHBuf[ANOTHERBUFSIZE], monthPBuf[ANOTHERBUFSIZE];
uint16_t tailHourBuf=0,tailDayBuf=0,tailWeekBuf=0,tailMonthBuf=0;
uint16_t dayPageCounter =0,weekPageCounter =0,monthPageCounter =0;
uint16_t ADC1ConvertedValue = 0;

ErrorStatus HSEStartUpStatus;
struct bme68x_dev bme;
struct bme68x_data bmeData;
float iaq=0;
extern uint8_t itsTimeFlag;


uint16_t getSr(uint16_t mul, uint16_t *buf){
  uint32_t sum=0;
  for(int i=0;i<mul;i++)
    sum+=buf[tailHourBuf-i-1];

  return sum/mul;
}
int main()
{
  uint8_t status;
  SystemInit();
  SysTick_Config(SystemCoreClock/1000);//1ms
  tim4_init();
  tim2_init();
  LEDInit();   
  GPIO_init();     
  usart_init();
  I2C_EE_Init();
  bme680Init(&bme);
  

  status=ds18b20_init(DS18B20_Resolution_12_bit);
    if (status){
      while(1);
  } 

  initADC();
  while(1){
      
    if(itsTimeFlag){
      getTHPG(&bme,&bmeData);                                                     //Читаем bme680
      iaq=iaqCalc(bmeData.gas_resistance,bmeData.humidity);                             
      temper = ds18b20_get_temperature();                                       //Читаем ds18b20
     
      updateEEPROM();

      if (getNowPage()==0){
         sensorsDataSend();
      }
              
      Nextion_SetValue_Number("x0.val=",(uint32_t)(bmeData.temperature*10));
      //delay_1_ms(5000);                                                           //Делаем измерения раз в 6 секунд 
      LEDToggle();
      itsTimeFlag=0;
    }
  }    
}

void sensorsDataSend(){
    Nextion_SetValue_Number("x0.val=",(uint32_t)(bmeData.temperature*10));
    Nextion_SetValue_Number("x1.val=",temper); 
    Nextion_SetValue_Number("n0.val=",(uint32_t)bmeData.humidity);
    Nextion_SetValue_Number("n1.val=",(uint32_t)(bmeData.pressure*0.00750062));
    Nextion_SetValue_Number("n2.val=",(uint32_t)iaq);
  
}

void updateEEPROM(){
  hourT1Buf[tailHourBuf]=temper;                                                //Положили температурку
  hourT2Buf[tailHourBuf]=(uint16_t)(bmeData.temperature*10);
  hourHBuf[tailHourBuf]=(uint16_t)(bmeData.humidity*10);                        //Влажность  % *10
  hourPBuf[tailHourBuf++]=(uint16_t)(bmeData.pressure*0.00750062);              //мм.рт.ст
  
    if(tailHourBuf%24==0){                                                      //Прошло 24 измерения (144 секунды) и можно записать в буфер дня
      dayT1Buf[tailDayBuf] = getSr(24,hourT1Buf);                               //Записываем среднее от 24 последних измерений
      dayT2Buf[tailDayBuf] = getSr(24,hourT2Buf);
      dayHBuf[tailDayBuf] = getSr(24,hourHBuf);
      dayPBuf[tailDayBuf++] = getSr(24,hourPBuf);

      if(tailDayBuf==ANOTHERBUFSIZE){                                           //Буфер заполнился (144 минуты), пора записывать
        if(getFromEE(DAYT1STARTPAGE,60,&dayPageCounter)!=0) while(1);           //Читаем текущую страницу для записи данных по дню
        if(writePageEE(dayT1Buf,DAYT1STARTPAGE+dayPageCounter)) while(1);       //пишем часть (1/10) по дню
        if(writePageEE(dayT2Buf,DAYT2STARTPAGE+dayPageCounter)) while(1); 
        if(writePageEE(dayHBuf,DAYHSTARTPAGE+dayPageCounter)) while(1); 
        if(writePageEE(dayPBuf,DAYPSTARTPAGE+dayPageCounter)) while(1); 
        dayPageCounter++;                                                       //Инкрементируем счетчик текущей страницы
        if(DAYT1STARTPAGE+dayPageCounter>DAYT1ENDTPAGE)  dayPageCounter=0;      //Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(DAYT1STARTPAGE,60,dayPageCounter)) while(1);                 //Записываем номер страницы для записи следующей пачки
        if(setToEE(DAYT2STARTPAGE,60,dayPageCounter)) while(1);
        if(setToEE(DAYHSTARTPAGE,60,dayPageCounter)) while(1);
        if(setToEE(DAYPSTARTPAGE,60,dayPageCounter)) while(1);
        
        tailDayBuf=0;
      }
    }
    
    if(tailHourBuf%168==0){                                                     //Прошло 168 измерений (1008 секунд) и можно записать в буфер недели
      weekT1Buf[tailWeekBuf] = getSr(168,hourT1Buf);                                      //Записываем среднее от 168 последних измерений
      weekT2Buf[tailWeekBuf] = getSr(168,hourT2Buf);
      weekHBuf[tailWeekBuf] = getSr(168,hourHBuf);
      weekPBuf[tailWeekBuf++] = getSr(168,hourPBuf);

      if(tailWeekBuf==ANOTHERBUFSIZE){                                           //Буфер заполнился (1008 минут - 16,8 часов), пора записывать
        if(getFromEE(WEEKT1STARTPAGE,60,&weekPageCounter)!=0) while(1);         //Читаем текущую страницу для записи данных по неделе
        if(writePageEE(weekT1Buf,WEEKT1STARTPAGE+weekPageCounter)) while(1);    //пишем часть (1/10) по неделе
        if(writePageEE(weekT2Buf,WEEKT2STARTPAGE+weekPageCounter)) while(1);
        if(writePageEE(weekHBuf,WEEKHSTARTPAGE+weekPageCounter)) while(1);
        if(writePageEE(weekPBuf,WEEKPSTARTPAGE+weekPageCounter)) while(1);
        
        weekPageCounter++;
        if(WEEKT1STARTPAGE+weekPageCounter>WEEKT1ENDTPAGE)  weekPageCounter=0;  //Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(WEEKT1STARTPAGE,60,weekPageCounter)) while(1);                 //Записываем номер страницы для записи следующей пачки
        if(setToEE(WEEKT2STARTPAGE,60,weekPageCounter)) while(1);
        if(setToEE(WEEKHSTARTPAGE,60,weekPageCounter)) while(1);
        if(setToEE(WEEKPSTARTPAGE,60,weekPageCounter)) while(1);
        
        tailWeekBuf=0;
      }
    }
    
    if(tailHourBuf==HOURBUFSIZE){                                                //Буфер заполнился, пора записывать
      monthT1Buf[tailMonthBuf] = getSr(600,hourT1Buf);                          //За 600 измерений прошел час. Записываем точку в буфер месяца. 600 таких точек дадут нам 25 дней.
      monthT2Buf[tailMonthBuf] = getSr(600,hourT2Buf);
      monthHBuf[tailMonthBuf] = getSr(600,hourHBuf);
      monthPBuf[tailMonthBuf++] = getSr(600,hourPBuf);

      for(int i=0;i<10;i++){
        if(writePageEE(&hourT1Buf[0]+i*60,i)) while(1);                         //Пишем все 6 частей
        if(writePageEE(&hourT2Buf[0]+i*60,i)) while(1);
        if(writePageEE(&hourHBuf[0]+i*60,i)) while(1);
        if(writePageEE(&hourPBuf[0]+i*60,i)) while(1);
      }
      if(setToEE(HOURT1STARTPAGE,60,0)) while(1);                               //Ставим указатель на нулевую страницу, потому что записываем сразу все 10 старниц
      tailHourBuf=0;
        
      if(tailMonthBuf==ANOTHERBUFSIZE){                                         //Буфер заполнился (60 часов), пора записывать
        if(getFromEE(MONTHT1STARTPAGE,60,&monthPageCounter)!=0) while(1);       //Читаем текущую страницу для записи данных по месяцу
        
        if(writePageEE(monthT1Buf,MONTHT1STARTPAGE+monthPageCounter)) while(1); //пишем часть (1/10) по месяцу
        if(writePageEE(monthT2Buf,MONTHT2STARTPAGE+monthPageCounter)) while(1);
        if(writePageEE(monthHBuf,MONTHHSTARTPAGE+monthPageCounter)) while(1);
        if(writePageEE(monthPBuf,MONTHPSTARTPAGE+monthPageCounter)) while(1);
        
        monthPageCounter++;
        if(MONTHT1STARTPAGE+monthPageCounter>MONTHT1ENDTPAGE)  monthPageCounter=0;//Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(MONTHT1STARTPAGE,60,monthPageCounter)) while(1);               //Записываем номер страницы для записи следующей пачки
        if(setToEE(MONTHT2STARTPAGE,60,monthPageCounter)) while(1);
        if(setToEE(MONTHHSTARTPAGE,60,monthPageCounter)) while(1);
        if(setToEE(MONTHPSTARTPAGE,60,monthPageCounter)) while(1);
        
        tailMonthBuf=0;
      }                                                                         //всего на 10 страниц влезет 600 часов - 25 дней.
    }    

}