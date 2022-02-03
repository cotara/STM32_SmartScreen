#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "user_USART.h"
#include "user_GPIO.h"
#include "user_TIMER.h"
#include "myNextion.h"
#include "user_ds18b20.h"
#include "LED_user.h"
#include "i2c_ee.h"

int errorSensor =0;
uint16_t temper;

uint8_t isEmpty=1;
uint16_t temper=0;
uint16_t tempBuf[512];
uint32_t average=0;

ErrorStatus HSEStartUpStatus;
void RCC_Configuration(void)
{
    /*RCC system reset(for debug purpose) */
    RCC_DeInit();

    RCC_HSICmd(DISABLE);
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK*/
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK*/
        RCC_PCLK1Config(RCC_HCLK_Div1);

        //ADC CLK
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        RCC->CFGR|= 0x01<<17;
          
        /* PLLCLK = 8MHz * 3 = 24 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }
}


int main()
{
  uint8_t status;
  SystemInit();
  
  
  //RCC_ClocksTypeDef RCC_Clocks;
  //RCC_GetClocksFreq(&RCC_Clocks);
  //RCC_Configuration();
  SysTick_Config(SystemCoreClock/1000);//1ms
  tim4_init();
  tim2_init();
  
  LEDInit();   
  GPIO_init();     
  usart_init();
  I2C_EE_Init();

/*
GPIO_ResetBits(GPIOA, GPIO_Pin_7);
GPIO_SetBits(GPIOA, GPIO_Pin_7);
GPIO_ResetBits(GPIOA, GPIO_Pin_7);
GPIO_SetBits(GPIOA, GPIO_Pin_7);
*/

  
 status=ds18b20_init(DS18B20_Resolution_9_bit);
    
    if (status){
      while(1);
    }
    
  while(1){
    if(getFLAG_END_LINE() == 3)
      nextionEvent();
    
    temper = ds18b20_get_temperature();
    setToEE(temper);
    
    if (getNowPage()==0){
     Nextion_SetValue_Number("x0.val=",temper);
       if(temper>=-300)
         Nextion_SetValue_Number("temperature.val=",temper*3/10+90);
       else
         Nextion_SetValue_Number("temperature.val=",temper*3/10+450);
     }
       
     else if(getNowPage()== 1){
       Nextion_SetValue_Number("add 2,0,",temper/2);
       
     }
     else if(getNowPage()== 2 && getReqBigBuf()){
       resetReqBigBuf();
       getFromEE(tempBuf,512);
       for(int i=0;i<512;i++){
        Nextion_SetValue_Number("add 1,0,",tempBuf[i]*0.667);
        average+=tempBuf[i];
       }
        Nextion_SetValue_Number("n0.val=",average/5120);
     }
       //delay_1_ms(500);
    }   
}
