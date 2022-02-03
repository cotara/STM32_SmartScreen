#include <stdint.h>

void delay_1_ms(volatile uint32_t nTime);
void TimingDelay_1ms_Decrement(void);
void delay_1_mcs(volatile uint32_t nTime);
void TimingDelay_1mcs_Decrement(void);

void tim4_init(void);
void tim2_init(void);