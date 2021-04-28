#include "stm32f10x_it.h"
#include "stm32f10x_usart.h"
#include "user_TIMER.h"
#include "user_USART.h"
#include "myNextion.h"
#define TX_BUFFER_SIZE 100

extern volatile uint8_t   tx_buffer[TX_BUFFER_SIZE];
extern volatile unsigned long  tx_wr_index,tx_rd_index,tx_counter;


void HardFault_Handler(void){
  while (1)
  {}
}

void MemManage_Handler(void){
  while (1)
  {}
}

void BusFault_Handler(void){
   while (1)
  {}
}


void SysTick_Handler(void){
  TimingDelay_Decrement();
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/
void TIM2_IRQHandler(void){
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TimingDelay_1mcs_Decrement();
    //setRxi(0);
}


void USART1_IRQHandler(void){
  if ((USART1->SR & USART_FLAG_RXNE))
   USART_IRQProcessFunc(USART_ReceiveData(USART1));    
  
  if(USART_GetITStatus(USART1, USART_IT_TC) == SET) {                          //прерывание по передаче
      if (tx_counter) {                                                           //если есть что передать
        --tx_counter;                                                             // уменьшаем количество не переданных данных
        USART_SendData(USART1,tx_buffer[tx_rd_index++]);                          //передаем данные инкрементируя хвост буфера
        if (tx_rd_index == TX_BUFFER_SIZE) tx_rd_index=0;                         //идем по кругу
      }
      else {
        USART_ITConfig(USART1, USART_IT_TC, DISABLE);                            //если нечего передать, запрещаем прерывание по передаче
      }
   } 
}

/*******************************************************************************/

