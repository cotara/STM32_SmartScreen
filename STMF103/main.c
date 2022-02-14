#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "user_USART.h"
#include "user_GPIO.h"
#include "user_TIMER.h"
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
uint16_t hourBuf[HOURBUFSIZE];                                                           //Хранит текущие отсчеты температуры (1р/6 сек)
uint16_t dayBuf[ANOTHERBUFSIZE];                                                          //Хранит считаныне 5 страниц EEPROM взятые для прореживания, например 1 час
uint16_t weekBuf[ANOTHERBUFSIZE];
uint16_t monthBuf[ANOTHERBUFSIZE];
uint16_t tailHourBuf=0,tailDayBuf=0,tailWeekBuf=0,tailMonthBuf=0;
uint16_t dayPageCounter =0,weekPageCounter =0,monthPageCounter =0;
uint32_t average=0;

ErrorStatus HSEStartUpStatus;
struct bme68x_dev bme;
struct bme68x_data bmeData;
float iaq=0;



uint16_t getSr(uint16_t mul){
  uint32_t sum=0;
  for(int i=0;i<mul;i++)
    sum+=hourBuf[tailHourBuf-i-1];

  return sum/mul;
}
int main()
{
  uint8_t status;
  uint16_t testNum=0;
  SystemInit();
  SysTick_Config(SystemCoreClock/1000);//1ms
  tim4_init();
  tim2_init();
  LEDInit();   
  GPIO_init();     
  usart_init();
  I2C_EE_Init();
  bme680Init(&bme);
    

  status=ds18b20_init(DS18B20_Resolution_9_bit);
    if (status){
      while(1);
  } 

  while(1){
    if(getFLAG_END_LINE() == 3)                                                 //пришла посылка от экранчика
      nextionEvent();
    
    //getTHPG(&bme,&bmeData);                                                                  //Читаем bme680
    //iaq=iaqCalc(bmeData.gas_resistance,bmeData.humidity);                             
    
    //temper = ds18b20_get_temperature();                                       //Читаем ds18b20
    temper=testNum++;
    
    hourBuf[tailHourBuf++]=temper;                                              //Положили температурку
    
    if(tailHourBuf%24==0){                                                      //Прошло 24 измерения (144 секунды) и можно записать в буфер дня
      dayBuf[tailDayBuf++] = getSr(24);                                         //Записываем среднее от 24 последних измерений
      if(tailDayBuf==ANOTHERBUFSIZE){                                            //Буфер заполнился (144 минуты), пора записывать
        if(getFromEE(DAYSTARTPAGE,60,&dayPageCounter)!=0) while(1);             //Читаем текущую страницу для записи данных по дню
        if(writePageEE(dayBuf,DAYSTARTPAGE+dayPageCounter)) while(1);           //пишем часть (1/10) по дню
        dayPageCounter++;                                                       //Инкрементируем счетчик текущей страницы
        if(DAYSTARTPAGE+dayPageCounter>DAYENDTPAGE)  dayPageCounter=0;          //Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(DAYSTARTPAGE,60,dayPageCounter)) while(1);                   //Записываем номер страницы для записи следующей пачки
        tailDayBuf=0;
      }
    }
    
    if(tailHourBuf%168==0){                                                     //Прошло 168 измерений (1008 секунд) и можно записать в буфер недели
      weekBuf[tailWeekBuf++] = getSr(168);                                      //Записываем среднее от 168 последних измерений
      if(tailWeekBuf==ANOTHERBUFSIZE){                                           //Буфер заполнился (1008 минут - 16,8 часов), пора записывать
        if(getFromEE(WEEKSTARTPAGE,60,&weekPageCounter)!=0) while(1);           //Читаем текущую страницу для записи данных по неделе
        if(writePageEE(weekBuf,WEEKSTARTPAGE+weekPageCounter)) while(1);        //пишем часть (1/10) по неделе
        weekPageCounter++;
        if(WEEKSTARTPAGE+weekPageCounter>WEEKENDTPAGE)  weekPageCounter=0;//Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(WEEKSTARTPAGE,60,weekPageCounter)) while(1);                 //Записываем номер страницы для записи следующей пачки
        tailWeekBuf=0;
      }
    }
    
    if(tailHourBuf==HOURBUFSIZE){                                                //Буфер заполнился, пора записывать
      monthBuf[tailMonthBuf++] = getSr(600);                                    //За 600 измерений прошел час. Записываем точку в буфер месяца. 600 таких точек дадут нам 25 дней.
      for(int i=0;i<10;i++){
        if(writePageEE(&hourBuf[0]+i*60,i)) while(1);                          //Пишем все 6 частей
      }
      tailHourBuf=0;
        
      if(tailMonthBuf==ANOTHERBUFSIZE){                                          //Буфер заполнился (60 часов), пора записывать
        if(getFromEE(MONTHSTARTPAGE,60,&monthPageCounter)!=0) while(1);         //Читаем текущую страницу для записи данных по месяцу
        if(writePageEE(monthBuf,MONTHSTARTPAGE+monthPageCounter)) while(1);     //пишем часть (1/10) по месяцу
        monthPageCounter++;
        if(MONTHSTARTPAGE+monthPageCounter>MONTHENDTPAGE)  monthPageCounter=0;     //Если записали последнюю (10-ю) часть, начинаем с первой страницы
        if(setToEE(MONTHSTARTPAGE,60,monthPageCounter)) while(1);               //Записываем номер страницы для записи следующей пачки
        tailMonthBuf=0;
      }                                                                         //всего на 10 страниц влезет 600 часов - 25 дней.
    }    
    
    
    
    /*
    if (getNowPage()==0){
       Nextion_SetValue_Number("x0.val=",(uint32_t)bmeData.temperature*10);
       Nextion_SetValue_Number("x1.val=",temper); 
       Nextion_SetValue_Number("n0.val=",(uint32_t)bmeData.humidity);
       Nextion_SetValue_Number("n1.val=",(uint32_t)(bmeData.pressure*0.00750062));
       Nextion_SetValue_Number("n2.val=",(uint32_t)iaq);
    }
       
     else if(getNowPage()== 1){
       Nextion_SetValue_Number("add 2,0,",temper/2);
       
     }
     else if(getNowPage()== 2 && getReqBigBuf()){
       resetReqBigBuf();
       //getFromEE(tempBuf,512);
       for(int i=0;i<512;i++){
        Nextion_SetValue_Number("add 1,0,",(uint32_t)(tempBuf[i]*0.667));
        average+=tempBuf[i];
       }
        Nextion_SetValue_Number("n0.val=",average/5120);
     }
     
    */
    //delay_1_ms(2);                                                           //Делаем измерения раз в 6 секунд
    
  
  
  }   

  
  
}
