#include <stdint.h>

void USART_IRQProcessFunc(uint8_t);
void nextionEvent(void);
void Nextion_SetValue_Number(char *ValueName, uint32_t Value);
void Nextion_SetValue_String(char *ValueName, char *Value);
uint8_t getFLAG_END_LINE(void);