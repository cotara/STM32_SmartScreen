#include "user_ds18b20.h"
#include "user_TIMER.h"

static uint32_t convert_delay = DELAY_T_CONVERT;

static void write_bit(const uint8_t bit);
static void write_byte(const uint8_t byte);
static uint8_t read_bit(void);
static uint8_t get_devider(const DS18B20_RESOLUTION_t resolution);
static uint16_t read_temperature(void);
static void reset(void);

void ds18b20_init(const DS18B20_RESOLUTION_t resolution) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // 50MHz output open-drain
    GPIOA->CRL |= GPIO_CRL_MODE1;
    GPIOA->CRL |= GPIO_CRL_CNF1_0;
    GPIOA->CRL &= ~GPIO_CRL_CNF1_1;

    ds18b20_set_resolution(resolution);
}
void ds18b20_set_resolution(const DS18B20_RESOLUTION_t resolution) {
    reset();                      // send reset
    write_byte(SKIP_ROM);         // work only with one device (пропуск команды определения адреса)
    write_byte(WRITE_SCRATCHPAD); // set resolution
    write_byte(TH_REGISTER);      //
    write_byte(TL_REGISTER);      //
    write_byte(resolution);       //
    convert_delay = DELAY_T_CONVERT / get_devider(resolution); // calc conversation delay
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
void reset(void) {
    GPIOA->ODR &= ~GPIO_ODR_ODR1;
    delay_1_mcs(DELAY_RESET);
    GPIOA->ODR |= GPIO_ODR_ODR1;
    delay_1_mcs(DELAY_RESET);
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
    GPIOA->ODR &= ~GPIO_ODR_ODR1;
    delay_1_mcs(bit ? DELAY_WRITE_1 : DELAY_WRITE_0);
    GPIOA->ODR |= GPIO_ODR_ODR1;
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
    GPIOA->ODR &= ~GPIO_ODR_ODR1;
    delay_1_mcs(DELAY_READ_SLOT);
    GPIOA->ODR |= GPIO_ODR_ODR1;
    // ... switch to INPUT
    delay_1_mcs(DELAY_BUS_RELAX);
    bit = (GPIOA->IDR & GPIO_IDR_IDR1 ? 1 : 0);
    delay_1_mcs(DELAY_READ_PAUSE);
    // ... switch to OUTPUT
    return bit;
}

uint16_t read_temperature(void) {
    uint16_t data = 0;
    for (uint8_t i = 0; i < 16; i++)
        data += (uint16_t) read_bit() << i;
    return (uint16_t)(((float) data / 16.0f) * 10.0f);
}