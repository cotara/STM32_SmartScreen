#include "stm32f10x_it.h"
#include "stm32f10x_usart.h"
#include "user_TIMER.h"
#include "user_USART.h"
#include "LED_user.h"
#include "myNextion.h"

extern uint16_t adcBuf[200];
extern uint16_t ADC1ConvertedValue;
extern volatile uint8_t   tx_buffer[TX_BUFFER_SIZE];
uint32_t mcs=0;
uint32_t m_ms=0;
uint8_t itsTimeFlag=0;
uint32_t sum=0;

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
  if(m_ms%500 ==0){
    for(int i=0;i<200;i++)
      sum+=adcBuf[i];
    ADC1ConvertedValue=sum/200;
    sum=0;
    if(getAutoBr() == 1)
      Nextion_SetValue_Number("dim=",(uint32_t)(ADC1ConvertedValue*0.024)+1);
  }
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/
void TIM2_IRQHandler(void){
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TimingDelay_1mcs_Decrement();
    mcs++;
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
          uint8_t RXc = USART_ReceiveData(USART1) & 0xFF;     
          toBuf(RXc);                                                           //кладем принятый символ в буфер
          TIM_Cmd(TIM4, DISABLE);                                               //Выключаем сервисный таймер юарта
          TIM4->CNT = 0;
          if (RXc == 0xFF)                                                      //Если пришел признак окончания команды (их должно быть 3)
            incFLAG_END_LINE();
          if(getFLAG_END_LINE()!=3)                                             //Если команда пришла еще не полностью, запускаем таймер сброса
               TIM_Cmd(TIM4, ENABLE);                                           //Снова запускаем таймер и выходим из прерывания
          else{
            resetFLAG_END_LINE();                                               //Пришла вся команда
            if(getRxi()==7)                                                     //От экрана пришла именно команда, а не ошибка (4 байта)
              addCommand();                                                     //Добавляем команду в буфер команд на выполнение
            else 
              clear_RXBuffer();                                                 //Если пришла ошибка, просто очищаем входной буфер для следующей команды
          }
        }                                 
        else  USART_ReceiveData(USART1);
        
    }
           
    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET) {                      //прерывание по передаче
        USART_ClearITPendingBit(USART1, USART_IT_TC);                           //очищаем признак прерывания
        if (getTXcounter()) {                                                   //если есть что передать
            setTXcounter(getTXcounter()-1);                                     // уменьшаем количество не переданных данных
            USART_SendData(USART1, getByteFromUARTbuf());                       //передаем данные из кольцевого буфера
        }
        else
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);                       //если нечего передать, запрещаем прерывание по передаче
    }

}

/*******************************************************************************/

