#include "stm32f10x_it.h"
#include "stm32f10x_usart.h"
#include "user_TIMER.h"
#include "user_USART.h"
#include "LED_user.h"
#include "myNextion.h"


extern volatile uint8_t   tx_buffer[TX_BUFFER_SIZE];
extern volatile unsigned long  tx_wr_index,tx_rd_index,tx_counter;
uint32_t mcs=0;
uint32_t m_ms=0;
uint8_t itsTimeFlag=0;
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
  TimingDelay_1ms_Decrement();
  m_ms++;
  if(m_ms>=6000){
    m_ms=0;
    itsTimeFlag=1;
  }
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/
void TIM2_IRQHandler(void){
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TimingDelay_1mcs_Decrement();
    //mcs++;
    //LEDToggle();
}
 
void TIM4_IRQHandler(void){
    TIM_ClearFlag(TIM4, TIM_IT_Update);
    setRxi(0);                      //1 мс не приходило нового байта. Значит был затык
    resetFLAG_END_LINE();
    TIM_Cmd(TIM4, DISABLE);         //Перестали считать. Ждем следующей посылки.
    TIM4->CNT = 0;
}

void USART1_IRQHandler(void){
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); //очистка признака прерывания
        if ((USART1->SR & (USART_FLAG_FE | USART_FLAG_PE)) == 0) { //нет ошибок
            USART_IRQProcessFunc(USART_ReceiveData(USART1) & 0xFF);
        }                                 
        else  USART_ReceiveData(USART1);
        
    }
           
    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET) { //прерывание по передаче
        USART_ClearITPendingBit(USART1, USART_IT_TC); //очищаем признак прерывания
        if (tx_counter) {                               //если есть что передать
            --tx_counter;           // уменьшаем количество не переданных данных
            USART_SendData(USART1, tx_buffer[tx_rd_index++]); //передаем данные инкрементируя хвост буфера
            if (tx_rd_index == TX_BUFFER_SIZE)
                tx_rd_index = 0;                         //идем по кругу
        }
        else
            USART_ITConfig(USART1, USART_IT_TC, DISABLE); //если нечего передать, запрещаем прерывание по передаче
    }

}

/*******************************************************************************/

