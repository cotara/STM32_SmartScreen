#include <stdint.h>

void USART_IRQProcessFunc(uint8_t);
void nextionEvent(void);
void Nextion_SetValue_Number(char *ValueName, int32_t Value);
void Nextion_SetValue_String(char *ValueName, char *Value);
uint8_t getFLAG_END_LINE(void);
void resetFLAG_END_LINE(void);
void incFLAG_END_LINE(void);
uint8_t getNowPage(void);
void readGrapgData(int16_t par, uint16_t *buf);
void transparent(uint16_t *buf);
void errorProcess(uint8_t code);
uint8_t getAutoBr();


void addCommand();
uint8_t getCountCommand();