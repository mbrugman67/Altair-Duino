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

volatile static uint32_t db0Counter = 0;
volatile static uint32_t db1Counter = 0;

static int heartbeatDoorbellID = 0;
static bool db0Started = false;
static bool db1Started = false;

void doorbellIRQCore0()
{
    ++db0Counter;
    multicore_doorbell_clear_current_core(heartbeatDoorbellID);
    log->dbgWrite("ISR\r\n");
}

void heartbeatCore0()
{
    static uint32_t lastCoreCount = db0Counter;
    static uint32_t startTime = 0;
    static bool triggered = false;
    static bool init = false;
    static bool alerted = false;

    if (!db0Started)
    {
        heartbeatDoorbellID = multicore_doorbell_claim_unused(3, true);

        uint32_t irqNum = multicore_doorbell_irq_num(heartbeatDoorbellID);
        irq_add_shared_handler(irqNum, doorbellIRQCore0, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        log->dbgWrite(stringFormat("HB ID %d, irq %d\r\n", heartbeatDoorbellID, irqNum));

        multicore_doorbell_clear_current_core(heartbeatDoorbellID);

        irq_set_enabled(irqNum, true);

        db0Started = true;
        startTime = to_ms_since_boot(get_absolute_time());
        return;
    }

    if (db0Started && db1Started)
    {
        if (!init)
        {
            lastCoreCount = db0Counter + 1;
            log->infoWrite("Starting watchdog\r\n");
            init = true;
        }

        if (db0Counter != lastCoreCount)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
            startTime = to_ms_since_boot(get_absolute_time());
            log->dbgWrite(stringFormat("Doorbell counter %d\r\n", db0Counter));
            triggered = false;
        }

        if ((to_ms_since_boot(get_absolute_time()) - startTime) > WATCHDOG_TIMEOUT)
        {
            if (!alerted)
            {
                log->errWrite("WATCHDOG EXPIRED!\r\n");
                alerted = true;
            }
        }
        else if ((to_ms_since_boot(get_absolute_time()) - startTime) > CORE_0_WATCHDOG_TIME)
        {
            if (!triggered)
            {
                multicore_doorbell_set_other_core(heartbeatDoorbellID);
                log->dbgWrite("Ringing doorbell\r\n");
                triggered = true;
            }
        }
    }

    lastCoreCount = db0Counter;
}

void doorbellIRQCore1()
{
    ++db1Counter;
    multicore_doorbell_clear_current_core(heartbeatDoorbellID);
    log->dbgWrite("ISR\r\n");
}

void heartbeatCore1()
{
    static uint32_t lastCoreCount = db1Counter;
    static uint32_t startTime = 0;
    static bool triggered = false;
    static bool alerted = false;

    if (db0Started && !db1Started)
    {
        uint32_t irqNum = multicore_doorbell_irq_num(heartbeatDoorbellID);
        irq_add_shared_handler(irqNum, doorbellIRQCore1, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        log->dbgWrite(stringFormat("HB ID %d, irq %d\r\n", heartbeatDoorbellID, irqNum));

        multicore_doorbell_clear_current_core(heartbeatDoorbellID);

        irq_set_enabled(irqNum, true);

        db1Started = true;
        startTime = to_ms_since_boot(get_absolute_time());
        return;
    }

    if (db0Started && db1Started)
    {
        if (db1Counter != lastCoreCount)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
            startTime = to_ms_since_boot(get_absolute_time());
            log->dbgWrite(stringFormat("Doorbell counter %d\r\n", db1Counter));
            triggered = false;
        }

        if ((to_ms_since_boot(get_absolute_time()) - startTime) > WATCHDOG_TIMEOUT)
        {
            if (!alerted)
            {
                log->errWrite("WATCHDOG EXPIRED!\r\n");
                alerted = true;
            }
        }
        else if ((to_ms_since_boot(get_absolute_time()) - startTime) > CORE_1_WATCHDOG_TIME)
        {
            if (!triggered)
            {
                multicore_doorbell_set_other_core(heartbeatDoorbellID);
                log->dbgWrite("Ringing doorbell\r\n");
                triggered = true;
            }
        }
    }

    lastCoreCount = db1Counter;
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
