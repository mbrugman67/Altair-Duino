/**********************************************************************
*
***********************************************************************/

// Pico specific includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

//
#include <cstdio>

int main(int argc, char* argv[])
{
    // init STDIN/STDOUT through USB
    stdio_init_all();

    // Wireless module init, no RTOS
    cyw43_arch_init();


    while(true)
    {
        sleep_ms(334);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(666);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    }
}
