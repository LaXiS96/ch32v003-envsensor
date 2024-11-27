#include <system.h>
#include <debug.h>
#include <gpio.h>
#include <mini-printf.h>
#include "i2c_int.h"
#include "bme280.h"

#define PIN_LED PD4

#define GPIO_IN_ANALOG 0b0000
#define GPIO_IN_FLOAT 0b0100
#define GPIO_IN_PUPD 0b1000
#define GPIO_OUT_PP 0b0011
#define GPIO_OUT_OD 0b0111
#define GPIO_OUT_ALT_PP 0b1011
#define GPIO_OUT_ALT_OD 0b1111

int main(void)
{
    SetupDebugPrintf();
    dbg_setup();

    RCC->APB1PCENR |= RCC_I2C1EN;
    RCC->APB2PCENR |= RCC_IOPCEN | RCC_AFIOEN;

    // Set PC1 and PC2 as alternate function, open-drain
    GPIOC->CFGLR |= GPIO_OUT_ALT_OD << (4 * 1) | GPIO_OUT_ALT_OD << (4 * 2);
    i2c_int_init();
    // bme280_init();

    I2C_cmd_t cmd = {
        .address = 0x76,
        .write_data = (uint8_t[]){0x88},
        .write_len = 1,
        .read_data = (uint8_t[25]){},
        .read_len = 25,
    };
    i2c_start(cmd);
    while (!i2c_ready())
        ;

    PIN_output(PIN_LED);

    char buf[128];
    while (1)
    {
        int32_t temp = 0;
        uint32_t press = 0;
        uint32_t hum = 0;
        // bme280_measure(&temp, &press, &hum);

        // int len = snprintf(buf, sizeof(buf),
        //                    "temp: %d.%u press: %u.%u hum: %u.%u\n",
        //                    (int32_t)(temp / 100.0), (uint32_t)(temp % 100),
        //                    (uint32_t)(press / 256.0), (uint32_t)(press % 256),
        //                    (uint32_t)(hum / 1024.0), (uint32_t)(hum % 1024));
        // _write(0, buf, len); // TODO wrap

        PIN_toggle(PIN_LED);
        DLY_ms(2000);
    }
}
