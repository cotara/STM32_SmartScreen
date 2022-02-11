
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bme68x.h"
#include "userBME680.h"
#include "user_TIMER.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
/******************************************************************************/
/*!                 Macro definitions                                         */
/*! BME68X shuttle board ID */
#define BME68X_SHUTTLE_ID  0x93
#define CS_LOW GPIO_ResetBits(GPIOA, GPIO_Pin_3);
#define CS_HIGH GPIO_SetBits(GPIOA, GPIO_Pin_3);
/******************************************************************************/
/*!                Static variable definition                                 */
static uint8_t dev_addr;

/******************************************************************************/
/*!                User interface functions                                   */



/*!
 * SPI read function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme68x_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;

    spiReadMulti(reg_addr, reg_data, len);
    return 0;
}

/*!
 * SPI write function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme68x_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    spiWriteMulti(reg_addr,(uint8_t*)reg_data,len);
    return 0;
}

/*
BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{    
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    uint8_t *data = reg_data;
    uint8_t start_reg_addr=reg_addr;
    uint32_t m_len = len;
    
    while(m_len--)
     *(uint8_t *)data++ =  I2C_ByteRead(start_reg_addr++,dev_addr);
    
    return 0;
}

BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    uint8_t start_reg_addr=reg_addr;
    uint32_t m_len = len;
    uint8_t byte;
    
    while(m_len--){
      byte = *(reg_data + (len-m_len));
      I2C_ByteWrite(byte,start_reg_addr++,dev_addr);
    }
    
    return 0;
}
*/
/*!
 * Delay function map to COINES platform
 */
void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
    delay_1_mcs(period);
}

void bme68x_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:

            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            printf("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            printf("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            printf("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            printf("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            printf("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            printf("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

int8_t bme68x_interface_init(struct bme68x_dev *bme)
{
    // Initialization struct
    SPI_InitTypeDef SPI_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
 
    // Step 1: Initialize SPI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStruct); 
    SPI_Cmd(SPI1, ENABLE);
 
    // Step 2: Initialize GPIO
    // GPIO pins for MOSI, MISO, and SCK
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    // GPIO pin for SS
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    CS_HIGH
              
    int8_t rslt = BME68X_OK;

    dev_addr = BME68X_I2C_ADDR_HIGH;
    bme->read = bme68x_spi_read;
    bme->write = bme68x_spi_write;
    bme->intf = BME68X_SPI_INTF;

    bme->delay_us = bme68x_delay_us;
    bme->intf_ptr = &dev_addr;
    bme->amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    return rslt;
    
    spiTransferByte(0x56);
}

// Transfer a byte over SPI2 B12/SS, B13/SCK, B14/MISO, B15/MOSI
uint8_t spiTransferByte(uint8_t outByte){
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, outByte); // send
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    return SPI_I2S_ReceiveData(SPI1); // read recieived
}

void spiSetReg(uint8_t addr, uint8_t byte){
    
  addr&=0x7F;
  CS_LOW
  spiTransferByte(addr);                                            
  spiTransferByte(byte);
  CS_HIGH                                           
}

uint8_t spiGetReg(uint8_t addr){
   uint8_t res;
   addr|=0x80;
  
   CS_LOW
   res = spiTransferByte(addr);                                              // send
   res = spiTransferByte(0xFF);
   CS_HIGH
     
   return res;
}


void  spiWriteMulti(uint8_t addr, uint8_t *buf, uint8_t len){
  uint8_t byte; 
  addr&=0x7f;
  CS_LOW
    spiTransferByte(addr);
    while (len--){
        byte = *buf;// send
        spiTransferByte(byte);
        buf++;
    }
   CS_HIGH
}

void spiReadMulti(uint8_t addr, uint8_t* buf, uint8_t len){
  uint8_t res;
  addr|=0x80;
  
  CS_LOW
    spiTransferByte(addr);                                                    // send
  while (len--) { 
      res = spiTransferByte(0xFF); 
      *(buf) =  res;  
      buf++;
  }
  CS_HIGH
}
float iaqCalc(float resistance, float hum){
    float hum_weighting = 0.25; // so hum effect is 25% of the total air quality score
    float gas_weighting = 0.75; // so gas effect is 75% of the total air quality score
    float hum_score,gas_score;

    float hum_reference = 40;
  
  //Calculate humidity contribution to IAQ index
  if (hum >= 38.0 && hum <= 42.0)
    hum_score = 25; // Humidity +/-5% around optimum 
  else
  { //sub-optimal
    if (hum < 38) 
      hum_score = hum_weighting/hum_reference*hum*100;
    else
    {
      hum_score = ((-hum_weighting/(100-hum_reference)*hum)+0.416666)*100;
    }
  }
  
  //Calculate gas contribution to IAQ index
  float gas_lower_limit = 10000;   // Bad air quality limit
  float gas_upper_limit = 300000;  // Good air quality limit 
  if (resistance > gas_upper_limit) resistance = gas_upper_limit; 
  if (resistance < gas_lower_limit) resistance = gas_lower_limit;
  
  gas_score = (gas_weighting/(gas_upper_limit-gas_lower_limit)*resistance -(gas_lower_limit*(gas_weighting/(gas_upper_limit-gas_lower_limit))))*100;
  
  //Combine results for the final IAQ index value (0-100% where 100% is good quality air)
  return hum_score + gas_score;
}