#include "user_USART.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#define RX_BUF_SIZE 80
#define TX_BUFFER_SIZE 100

uint8_t RXi;
uint8_t RX_BUF[RX_BUF_SIZE] = {'\0'};

// UART TX BUFFER
volatile uint8_t   tx_buffer[TX_BUFFER_SIZE];
volatile unsigned long  tx_wr_index=0,
                   tx_rd_index=0,
                   tx_counter=0;

void usart_init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
        

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
		USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        
        /* Enable the USARTx Interrupt */
	
        NVIC_SetPriority (USART1_IRQn, 0);
        NVIC_EnableIRQ (USART1_IRQn);
}
void clear_RXBuffer(void) {
    for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
        RX_BUF[RXi] = '\0';
    RXi = 0;
}

void setRxi(uint16_t i){
  RXi = i;
}
uint8_t getRxi(void){
  return RXi;
}

uint8_t toBuf(uint8_t n){
  if(RXi<RX_BUF_SIZE){
    RX_BUF[RXi]=n;
    RXi++;
    return 0;
  }
  else
   return 1;
}
uint8_t fromBuf( uint16_t i){
    return RX_BUF[i];
}

void USART1_put_char(uint8_t c) {
  while (tx_counter == TX_BUFFER_SIZE);                                         //если буфер переполнен, ждем
  USART_ITConfig(USART1, USART_IT_TC, DISABLE);                                 //запрещаем прерывание, чтобы оно не мешало менять переменную
  if (tx_counter || (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET))     //если в буфере уже что-то есть или если в данный момент что-то уже передается
  {
    tx_buffer[tx_wr_index++] = c;                                               //то кладем данные в буфер
    if (tx_wr_index == TX_BUFFER_SIZE) tx_wr_index=0;                           //идем по кругу
    ++tx_counter;                                                               //увеличиваем счетчик количества данных в буфере
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);                                //если UART свободен
  }
  else                                                                          //передаем данные без прерывания
    USART_SendData(USART1, c);                                                  //разрешаем прерывание
}

void USART1_put_string(unsigned char *string, uint32_t l) {
  while (l != 0){
    USART1_put_char(*string++);
    l--;
  }
}