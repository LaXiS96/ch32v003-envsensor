#include <system.h>
#include <gpio.h>

#define PIN_LED PD4

int main(void)
{
    PIN_output(PIN_LED);

    while (1)
    {
        PIN_toggle(PIN_LED);
        DLY_ms(200);
    }
}
