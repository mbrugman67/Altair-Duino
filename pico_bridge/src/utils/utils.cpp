// Pico includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"

// Normal C/C++ includes
#include <cstdio>

// project includes
#include "mlogger.h"
#include "stringFormat.h"

void heartbeat()
{
    static uint32_t startTime = to_ms_since_boot(get_absolute_time());


    uint32_t elapsedTime = to_ms_since_boot(get_absolute_time()) - startTime;

    // blink the LED with a 1Hz frequency 66% duty cycle
    if (elapsedTime < 666)          cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    else if (elapsedTime < 1000)    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    else                            startTime = to_ms_since_boot(get_absolute_time());
}

void heartbeat1()
{
    static uint32_t startTime = to_ms_since_boot(get_absolute_time());
    static uint32_t count = 0;
    static logger* log = logger::getInstance();

    uint32_t elapsedTime = to_ms_since_boot(get_absolute_time()) - startTime;

    // blink the LED with a 1Hz frequency 66% duty cycle
    if (elapsedTime > 1000)
    {
        log->dbgWrite(stringFormat("heartbeat count %d\r\n", count++).c_str());
        startTime = to_ms_since_boot(get_absolute_time());
    }
}

void chkForReset()
{
    logger* log = logger::getInstance();

    if (getchar_timeout_us(0) == 'r')
    {
        log->dbgWrite("Got a reset\r\n");

        // flush stdio before restarting
        printf("%s", log->pop().c_str());

        // turn on watchdog with a 50ms delay
        watchdog_enable(50, 0);
        while (true);
    }
}
