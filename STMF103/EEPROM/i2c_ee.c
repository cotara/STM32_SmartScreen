#include "i2c_ee.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_rcc.h"
#include "user_TIMER.h"


#define EEPROM_HW_ADDRESS      0xA0   /* E0 = E1 = E2 = 0 0x50<<1*/

void Delay_ms(uint32_t ms);

/***************************************************************************//**
 *  @brief  I2C Configuration
 ******************************************************************************/
void I2C_EE_Init(void)
{
   	   I2C_InitTypeDef  I2C_InitStruct;
	   GPIO_InitTypeDef  GPIO_InitStructure;

	   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
           RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
           
          // настройка I2C
          I2C_InitStruct.I2C_ClockSpeed = 400000; 
          I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
          I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
          // адрес отфанарный
          I2C_InitStruct.I2C_OwnAddress1 = 0x01;
          I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
          I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
          
            I2C_Init(I2C1, &I2C_InitStruct);          
          // I2C использует две ноги микроконтроллера, их тоже нужно настроить
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
          GPIO_Init(GPIOB, &GPIO_InitStructure);
 
          // стартуем модуль I2C
          I2C_Cmd(I2C1, ENABLE);      
}

//*******************************************************************8
//***************************************************************

void I2C_EE_ByteWrite(uint8_t val, uint16_t WriteAddr)
{
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


    /* Send the EEPROM's internal address to write to : MSB of the address first */
    I2C_SendData(I2C1, (uint8_t)((WriteAddr & 0xFF00) >> 8));
    //I2C_SendData(I2C1, (uint8_t)(0));
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send the EEPROM's internal address to write to : LSB of the address */
    I2C_SendData(I2C1, (uint8_t)(WriteAddr & 0x00FF));
      //I2C_SendData(I2C1, WriteAddr);
    /* Test on EV8 and clear it */
    while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

     I2C_SendData(I2C1, val);

        /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send STOP condition */
    I2C_GenerateSTOP(I2C1, ENABLE);

    //delay between write and read...not less 4ms
    delay_1_ms(5);
}
//*********************************************************************************
uint8_t I2C_EE_ByteRead( uint16_t ReadAddr)
{
    uint8_t tmp;

	/* While the bus is busy */
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


    /* Send the EEPROM's internal address to read from: MSB of the address first */
    I2C_SendData(I2C1, (uint8_t)((ReadAddr & 0xFF00) >> 8));
    //I2C_SendData(I2C1, (uint8_t)(0));
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send the EEPROM's internal address to read from: LSB of the address */
    I2C_SendData(I2C1, (uint8_t)(ReadAddr & 0x00FF));
    //I2C_SendData(I2C1, ReadAddr);
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Receiver);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));//I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    tmp=I2C_ReceiveData(I2C1);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C1, ENABLE);

    return tmp;
}
//Примем, что на странице всего 60 мест (с 0 по 59) для двухбайтных регистров
//60-е место оставляем для служебной информации

uint8_t setToEE(uint16_t page, uint16_t place, uint16_t val){

  uint8_t msb,lsb;
  uint8_t temp1,temp2;
  uint16_t memAdd = page*128+place*2;
  
  if(place>60)
    return 1;
  
  lsb=(uint8_t)(val & 0x00FF);
  msb=(uint8_t)((val & 0xFF00)>> 8);

  I2C_EE_ByteWrite(msb,memAdd++);delay_1_ms(5);
  I2C_EE_ByteWrite(lsb,memAdd++);delay_1_ms(5);

  
  temp1=I2C_EE_ByteRead(memAdd-2);
  temp2=I2C_EE_ByteRead(memAdd-1);
 
  
  if( temp1==msb && temp2==lsb)
         return 0;
  else
         return 1;
}

uint16_t getFromEE(uint16_t page, uint16_t place,  uint16_t *data){
  
  uint16_t memAdd = page*128+place*2;
  uint8_t msb,lsb;
  if(place>60)
    return 1;
  msb=I2C_EE_ByteRead(memAdd++);
  lsb=I2C_EE_ByteRead(memAdd);
  
  *data = msb*256+lsb;
  return 0;
}


uint8_t writePageEE(uint16_t *val, uint16_t page){
  uint8_t msb,lsb;
  uint16_t addr;
  uint16_t temp_buf[60];
  uint16_t *_val=val;
  addr = page*128;
    
  I2C_GenerateSTART(I2C1, ENABLE);                                              while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Transmitter);      while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1, (uint8_t)((addr & 0xFF00) >> 8));                          while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_SendData(I2C1, (uint8_t)(addr & 0x00FF));                                 while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  for(int i=0;i<60;i++){
    msb=(uint8_t)(((*_val) & 0xFF00)>> 8);
    lsb=(uint8_t)((*_val) & 0x00FF);
    _val++;
    I2C_SendData(I2C1, msb);                                                    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C1, lsb);                                                    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  }                                                                             

  //Send STOP condition 
  I2C_GenerateSTOP(I2C1, ENABLE);

  //delay between write and read...not less 5ms
  delay_1_ms(10);
  
  //Проверяем, что записалось верно  
  readPageEE(temp_buf,page);
  
  for(int i=0;i<60;i++){
    if(temp_buf[i] != *val++)
      return 1;
  }
    
  return 0;
}

uint8_t readPageEE(uint16_t *val1, uint16_t page){
  uint8_t msb,lsb;
  uint16_t addr;
  addr = page*128;

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
  I2C_GenerateSTART(I2C1, ENABLE);                                              while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Transmitter);      while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1, (uint8_t)((addr & 0xFF00) >> 8));                          while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_SendData(I2C1, (uint8_t)(addr & 0x00FF));                                 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1, ENABLE);                                              while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  
  
  I2C_Send7bitAddress(I2C1, EEPROM_HW_ADDRESS, I2C_Direction_Receiver);         
  
  for(int i=0;i<59;i++){
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
    msb=I2C_ReceiveData(I2C1);                                                  
    
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
    lsb=I2C_ReceiveData(I2C1);                                                  
    *val1 = msb*256 + lsb;
    val1++;
  }
  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb=I2C_ReceiveData(I2C1);                                                    
  
  I2C_AcknowledgeConfig(I2C1, DISABLE);
  while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb=I2C_ReceiveData(I2C1);
  *val1 = msb*256 + lsb;    
  
  // Send STOP condition
  I2C_GenerateSTOP(I2C1, ENABLE);

  return 0;
}

uint8_t erasePageEE(uint16_t page){
  uint16_t buf[60]={0};

  return writePageEE(buf,page);
}

uint8_t eraseEE(){
  uint16_t buf[60]={0};
  uint8_t res=0;
  
  for(int i=0;i<512;i++)
    res+=writePageEE(buf,i);
  
  if(res>0) return 1;
  else return 0;
}

