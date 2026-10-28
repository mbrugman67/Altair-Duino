/**********************************************************************
*
***********************************************************************/

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include <cstdio>

int main(int argc, char* argv[])
{
    stdio_init_all();
    cyw43_arch_init();

    while(true)
    {
        sleep_ms(334);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        printf("Turning off\r\n");
        sleep_ms(666);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
        printf("Turning on\r\n");
    }
}
