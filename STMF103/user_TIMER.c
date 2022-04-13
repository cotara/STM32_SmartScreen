#include "user_TIMER.h"
#include "stm32f10x_tim.h"

volatile uint32_t TimingDelay_1ms, TimingDelay_1mcs;

void tim2_init(void){
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 36-1;                                   //1 мкс
    TIMER_InitStructure.TIM_Period = 10-1;                                         
    TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
  
    NVIC_SetPriority (TIM2_IRQn, 0);
    NVIC_EnableIRQ (TIM2_IRQn);
}

//Работа ЮАРТА
void tim4_init(void){    
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 72-1;                                   //1 мс
    TIMER_InitStructure.TIM_Period = 1000-1;                                         
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    
    NVIC_SetPriority (TIM4_IRQn, 2);
    NVIC_EnableIRQ (TIM4_IRQn);
}

/*Задержка в nTime 1 мс*/
void delay_1_ms(volatile uint32_t nTime){
  TimingDelay_1ms = nTime;
  while(TimingDelay_1ms != 0);
}
void TimingDelay_1ms_Decrement(void){
  if (TimingDelay_1ms != 0x00)
    TimingDelay_1ms--;
}
/*Задержка в nTime 1 мкс*/
void delay_1_mcs(volatile uint32_t nTime)
{
  TimingDelay_1mcs = nTime;

  while(TimingDelay_1mcs != 0);
}
void TimingDelay_1mcs_Decrement(void)
{
  if (TimingDelay_1mcs != 0x00)
  { 
    TimingDelay_1mcs--;
  }
}