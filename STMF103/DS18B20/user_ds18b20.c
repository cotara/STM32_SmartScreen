#include "user_ds18b20.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "user_TIMER.h"

#define PIN_LOW GPIO_ResetBits(GPIOA, GPIO_Pin_7);
#define PIN_HIGH GPIO_SetBits(GPIOA, GPIO_Pin_7);
#define READ_DS18B20_BIT GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);

static uint32_t convert_delay = DELAY_T_CONVERT;

static void write_bit(const uint8_t bit);
static void write_byte(const uint8_t byte);
static uint8_t read_bit(void);
static uint8_t get_devider(const DS18B20_RESOLUTION_t resolution);
static uint16_t read_temperature(void);
static uint8_t reset(void);

uint8_t ds18b20_init(const DS18B20_RESOLUTION_t resolution) {

  GPIO_InitTypeDef GPIO_InitStructure;
  
    
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    return ds18b20_set_resolution(resolution);
}
uint8_t ds18b20_set_resolution(const DS18B20_RESOLUTION_t resolution) {
    if(reset())                   // send reset
        return 1;
    else{
      write_byte(SKIP_ROM);         // work only with one device 
      write_byte(WRITE_SCRATCHPAD); // set resolution
      write_byte(TH_REGISTER);      //
      write_byte(TL_REGISTER);      //
      write_byte(resolution);       //
      convert_delay = DELAY_T_CONVERT / get_devider(resolution); // calc conversation delay
    }
    return 0;
}

uint8_t get_devider(const DS18B20_RESOLUTION_t resolution) {
    uint8_t devider;
    switch (resolution) {
    case DS18B20_Resolution_9_bit:
        devider = 8;
        break;
    case DS18B20_Resolution_10_bit:
        devider = 4;
        break;
    case DS18B20_Resolution_11_bit:
        devider = 2;
        break;
    case DS18B20_Resolution_12_bit:
    default:
        devider = 1;
        break;
    }
    return devider;
}
uint8_t  reset(void) {
    uint8_t bit=1;
    PIN_LOW
    delay_1_mcs(DELAY_RESET);
    PIN_HIGH
    delay_1_mcs(DELAY_WRITE_0);
    
    bit = READ_DS18B20_BIT
    delay_1_mcs(DELAY_RESET);  

  return (bit ? 1 : 0);//вернём результат
}

uint16_t ds18b20_get_temperature() {
    reset();
    write_byte(SKIP_ROM);
    write_byte(CONVERT_T);
    delay_1_mcs(convert_delay);

    reset();
    write_byte(SKIP_ROM);
    write_byte(READ_SCRATCHPAD);

    return read_temperature();
}

void write_bit(uint8_t bit) {
    PIN_LOW
    delay_1_mcs(bit ? DELAY_WRITE_1 : DELAY_WRITE_0);
    PIN_HIGH
    delay_1_mcs(bit ? DELAY_WRITE_1_PAUSE : DELAY_WRITE_0_PAUSE);
}

void write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        write_bit(data >> i & 1);
        delay_1_mcs(DELAY_RELAXATION);
    }
}

uint8_t read_bit(void) {
    uint8_t bit = 0;
    PIN_LOW
    delay_1_mcs(DELAY_READ_SLOT);
    PIN_HIGH
    delay_1_mcs(DELAY_BUS_RELAX);
    bit = READ_DS18B20_BIT
    delay_1_mcs(DELAY_READ_PAUSE);

    return bit;
}

uint16_t read_temperature(void) {
    uint16_t data = 0;
    for (uint8_t i = 0; i < 16; i++)
        data += (uint16_t) read_bit() << i;
    return (uint16_t)(((float) data / 16.0f) * 10.0f);
}