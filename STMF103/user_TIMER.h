#include <stdint.h>

void Delay(volatile uint32_t nTime);
void TimingDelay_Decrement(void);
void delay_1_mcs(volatile uint32_t nTime);
void TimingDelay_1mcs_Decrement(void);

void tim4_init(void);
void tim3_init(void);