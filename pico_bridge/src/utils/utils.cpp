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

static int heartbeatDoorbellID0 = 0;
static int heartbeatDoorbellID1 = 0;
static bool db0Started = false;
static bool db1Started = false;
static bool trig0to1 = true;

void doorbellIRQCore0()
{
    ++db0Counter;
    multicore_doorbell_clear_current_core(heartbeatDoorbellID0);
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
        heartbeatDoorbellID0 = multicore_doorbell_claim_unused(3, true);

        uint32_t irqNum = multicore_doorbell_irq_num(heartbeatDoorbellID0);
        //irq_add_shared_handler(irqNum, doorbellIRQCore0, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_exclusive_handler(irqNum, doorbellIRQCore0);
        log->dbgWrite(stringFormat("HB ID %d, irq %d\r\n", heartbeatDoorbellID0, irqNum));

        multicore_doorbell_clear_current_core(heartbeatDoorbellID0);

        irq_set_enabled(irqNum, true);

        db0Started = true;
        startTime = to_ms_since_boot(get_absolute_time());
        return;
    }

    if (db0Started)
    {
        if (db0Counter != lastCoreCount)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
            startTime = to_ms_since_boot(get_absolute_time());
            log->dbgWrite(stringFormat("Doorbell counter %d\r\n", db0Counter));
            triggered = false;
            lastCoreCount = db0Counter;
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
                trig0to1 = true;
                log->dbgWrite("Setting trig0to1\r\n");
                triggered = true;
            }
        }
    }

}

void doorbellIRQCore1()
{
    //++db1Counter;
    //multicore_doorbell_clear_current_core(heartbeatDoorbellID1);
    //log->dbgWrite("ISR\r\n");
}

void heartbeatCore1()
{
    static uint32_t startTime = 0;
    static bool triggered = false;
    static bool init = false;
    static bool alerted = false;

    if (db0Started)
    {
        if (trig0to1)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
            trig0to1 = false;
            startTime = to_ms_since_boot(get_absolute_time());
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
                multicore_doorbell_set_other_core(heartbeatDoorbellID0);
                log->dbgWrite("Ringing doorbell\r\n");
                triggered = true;
            }
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
