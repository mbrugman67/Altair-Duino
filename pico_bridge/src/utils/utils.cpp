// Pico includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

// Normal C/C++ includes
#include <cstdio>
#include <string>

// project includes
#include "../include/globals.h"
#include "stringFormat.h"
#include "mlogger.h"

static logger* log = logger::getInstance();

void heartbeat()
{
    static uint16_t other[2] = {1, 0};
    static uint32_t hbCount[2] = {0, 0};
    static uint32_t hbLastCount[2] = {0, 1};
    static uint32_t startTime[2] = {to_ms_since_boot(get_absolute_time()), to_ms_since_boot(get_absolute_time())};
    static uint32_t hbTime[2] = {CORE_0_WATCHDOG_TIME, CORE_1_WATCHDOG_TIME};
    static bool triggered[2] = {false, false};
    static bool ledState[2] = {true, false};
    static bool alerted[2] = {false, false};

    uint16_t core = get_core_num();
    uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - startTime[core];

    if (hbCount[core] != hbLastCount[core])
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledState[core]);
        startTime[core] = to_ms_since_boot(get_absolute_time());
        triggered[core] = false;
        hbLastCount[core] = hbCount[core];
    }

    if (elapsed > WATCHDOG_TIMEOUT)
    {
        if (!alerted[core])
        {
            log->errWrite("WATCHDOG EXPIRED!\r\n");
            alerted[core] = true;
        }
    }
    else if (elapsed > hbTime[core])
    {
        if (!triggered[core])
        {
            ++hbCount[other[core]];
            triggered[core] = true;
        }
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
