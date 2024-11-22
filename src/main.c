#include <system.h>
#include <debug.h>
#include <gpio.h>
#include <mini-printf.h>

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

    // TODO enable peripheral clocks
    GPIOC->CFGLR |= GPIO_OUT_ALT_OD << (4 * 1) | GPIO_OUT_ALT_OD << (4 * 2); // Set PC1 and PC2 as alternate function, open-drain

    PIN_output(PIN_LED);

    char buf[64];
    int i = 0;
    while (1)
    {
        int len = snprintf(buf, sizeof(buf), "i: %u\n", i++);
        _write(0, buf, len); // TODO wrap

        PIN_toggle(PIN_LED);
        DLY_ms(200);
    }
}
