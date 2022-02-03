#include "LED_user.h"
#include "user_TIMER.h"

void LEDInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}


void LEDOn(void)
{
  GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
void LEDOff(void)
{
  GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void LEDToggle(void)
{
  GPIOC->ODR ^= GPIO_Pin_13;
}
void Blink (int x, int on, int off)                      
{      
        while (x--)
        {
          LEDOn();
          delay_1_mcs (on);
          LEDOff();
          delay_1_mcs (off);
        }  
}
