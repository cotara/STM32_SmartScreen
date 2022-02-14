#ifndef __I2CEE_H__
#define __I2CEE_H__

#include <stdint.h>
#define TEMPER_MAX_BUF 256
void I2C_EE_Init(void);
void I2C_EE_ByteWrite(uint8_t val, uint16_t WriteAddr);
uint8_t I2C_EE_ByteRead( uint16_t ReadAddr);

uint8_t setToEE(uint16_t page, uint16_t place,uint16_t val);
uint16_t getFromEE(uint16_t page, uint16_t place,  uint16_t *data);


uint8_t readPageEE(uint16_t *val, uint16_t page);
uint8_t writePageEE(uint16_t *val, uint16_t page);
uint8_t erasePageEE(uint16_t page);
uint8_t eraseEE();


#endif /* __I2CEE_H__ */