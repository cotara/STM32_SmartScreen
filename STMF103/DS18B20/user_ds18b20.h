#ifndef __DS18B20_H__
#define __DS18B20_H__

#define DELAY_RESET           500
#define DELAY_WRITE_0         60
#define DELAY_WRITE_0_PAUSE   10
#define DELAY_WRITE_1         10
#define DELAY_WRITE_1_PAUSE   60
#define DELAY_READ_SLOT       10
#define DELAY_BUS_RELAX       10
#define DELAY_READ_PAUSE      50
#define DELAY_T_CONVERT       760000
#define DELAY_RELAXATION      5


//Преобразование для 12 бит = 750 мс

#include "stm32f10x.h"
//#include "utils.h"

typedef enum {
    DS18B20_Resolution_9_bit    = 0x1F,
    DS18B20_Resolution_10_bit   = 0x3F,
    DS18B20_Resolution_11_bit   = 0x5F,
    DS18B20_Resolution_12_bit   = 0x7F
} DS18B20_RESOLUTION_t;

typedef enum {
    SKIP_ROM         = 0xCC,
    CONVERT_T        = 0x44,
    READ_SCRATCHPAD  = 0xBE,
    WRITE_SCRATCHPAD = 0x4E,
    TH_REGISTER      = 0x4B,
    TL_REGISTER      = 0x46,
} COMMANDS_t;

void ds18b20_init(const DS18B20_RESOLUTION_t resolution);
void ds18b20_set_resolution(const DS18B20_RESOLUTION_t resolution);
uint16_t ds18b20_get_temperature(void);

#endif /* __DS18B20_H__ */