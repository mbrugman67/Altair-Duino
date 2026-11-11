// Pico includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

// Normal C/C++ includes
#include <cstdio>
#include <string>

// project includes
#include "../include/globals.h"
#include "mlogger.h"

static logger* log = logger::getInstance();

// Do formatting like snprintf() but for std::strings
// BIG NOTE:  YOU CANNOT PASS A STD::STRING as a token replacement;
// this uses vsnprintf() internally, which doesn't understand std::strings.
//
// Specifically DO NOT do something like this:
//
//  ......
//  std::string someString;
//  someString = "abcdef";
//
//  std::string result = stringFormat("Here's the result: %s\n", someString);
//  .......
//
// This will not give you a result like "Here's the result: abcdef"
// Instead, you'll probably have a runtime exception.  However, this
// would be totes legit:
//
//  std::string result = stringFormat("Here's the result: %s\n", someString.c_str());
//
std::string stringFormat(const std::string& fmt, ...)
{
    // start with a std::string raw buffer of 1k
    int size = 1024;
    std::string str;
    va_list ap;
    size_t retries = 0;

    while (true)
    {
        str.resize(size);
        va_start(ap, fmt);

        // try to format print into the raw buvver of the std::string
        ssize_t n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);

        // This means that the std::string was allocated big enough.  Trim
        // the std::string's buffer down and head out
        if (n > -1 && n < size)
        {
            str.resize(n);
            return (str);
        }

        // A non-negative return value indicates more data would've been
        // written had there been enough buffer.  So set the buffer size
        // to be one larger than the number needed and go try again.  Note that
        // the return value includes the trailing null
        if (n > -1)
        {
            size = n + 1;
        }
        // a negative return value indicates an error occurred.  Double the
        // size of the buffer and try again.  Give up after some arbitrary
        // number of tries
        else
        {
            if (++retries < 5)
            {
                size *= 2;
            }
            else
            {
                return std::string("");
            }

        }
    }
    return (str);
}


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

    char c = getchar_timeout_us(0);
    switch (c)
    {
        case 'r':
        {
            log->dbgWrite("Got a reset\r\n");

            // flush stdio before restarting
            printf("%s", log->pop().c_str());

            // turn on watchdog with a 50ms delay
            watchdog_enable(50, 0);
            while (true);
        }  break;

        case 'b':
        {
            log->dbgWrite("Requesting bootloader!\r\n");
            printf("%s", log->pop().c_str());

            rom_reset_usb_boot(0, 0);
        }  break;

        case 'i':
        {
            log->dbgWrite("Debug info....\r\n");
            log->dbgWrite("Pending ISRs:\r\n");
            log->rawWrite(stringFormat("\t->PIN_SELECT: %d\r\n", gpio_get_irq_event_mask(PIN_SELECT)));
            log->rawWrite(stringFormat("\t   ->PIN_INP: %d\r\n", gpio_get_irq_event_mask(PIN_INP)));
            log->rawWrite(stringFormat("\t   ->PIN_OUT: %d\r\n", gpio_get_irq_event_mask(PIN_OUT)));
            log->dbgWrite(stringFormat("Debug count = %d\r\n", dbgCount));
            log->dbgWrite(stringFormat("isrFail count = %d\r\n", isrFail));
            log->dbgWrite(stringFormat("ISR call count = %d\r\n\r\n", isrCount));
        }  break;
    } // user input switch
}
