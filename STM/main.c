#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "user_USART.h"
#include "user_GPIO.h"
#include "user_TIMER.h"
#include "myNextion.h"
#include "user_ds18b20.h"


int errorSensor =0;
uint16_t temper;
ErrorStatus HSEStartUpStatus;

/*
void RCC_Configuration(void)
{
    //RCC system reset(for debug purpose) 
    RCC_DeInit();

    RCC_HSICmd(DISABLE);
    // Enable HSE 
    RCC_HSEConfig(RCC_HSE_ON);

    // Wait till HSE is ready 
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        // HCLK = SYSCLK 
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        // PCLK2 = HCLK
        RCC_PCLK2Config(RCC_HCLK_Div1);

        // PCLK1 = HCLK
        RCC_PCLK1Config(RCC_HCLK_Div1);

        //ADC CLK
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        RCC->CFGR|= 0x01<<17;
          
        //PLLCLK = 8MHz * 3 = 24 MHz 
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_3);

        // Enable PLL 
        RCC_PLLCmd(ENABLE);

        // Wait till PLL is ready 
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        // Select PLL as system clock source 
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        // Wait till PLL is used as system clock source 
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }
}
*/

int main()
{
  SystemInit();

  //RCC_ClocksTypeDef RCC_Clocks;
  //RCC_GetClocksFreq(&RCC_Clocks);
  //RCC_Configuration();
  SysTick_Config(SystemCoreClock/3000);//1ms
     
  GPIO_init();     
  //usart_init();
  //tim4_init();
  tim3_init();

  while(1){
    if(getFLAG_END_LINE() == 3)
      nextionEvent();
    
    //ds18b20_init(DS18B20_Resolution_11_bit);
    //temper = ds18b20_get_temperature();
  }
}
