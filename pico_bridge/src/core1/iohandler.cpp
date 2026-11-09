
#include "hardware/gpio.h"

#include "../include/globals.h"
#include "../utils/mlogger.h"
#include "../utils/utils.h"

enum bus_event_t
{
    BUS_NONE,
    BUS_INPUT,
    BUS_OUTPUT,
    BUS_NO_SELECT,
    BUS_UNKNOWN
};

static bool ioInitd = false;
static logger* log = logger::getInstance();

static volatile bus_event_t busEvent = BUS_NONE;

void altairBusISR(uint gpio, uint32_t event_mask)
{
    if (!gpio_get(PIN_SELECT))
    {
        busEvent = BUS_NO_SELECT;
        return;
    }

    switch (gpio)
    {
        case PIN_INP:   busEvent = BUS_INPUT;   return;
        case PIN_OUT:   busEvent = BUS_OUTPUT;  return;
    }

    busEvent = BUS_UNKNOWN;
    return;
}

/**********************************************
* Dedicated inputs:
*   GP0 - Select
*   GP1 - A0
*   GP2 - A1
*   GP3 - INP
*   GP4 - OUT
*
* Switched outputs: (mostly Hi-Z, but used as output)
*   GP5 - Wait
**********************************************/
void initIO()
{
    uint32_t pin_mask = (1u << PIN_SELECT) | (1u << PIN_A0) | (1u << PIN_A1) |
                        (1u << PIN_INP) | (1u << PIN_OUT) | (1u << PIN_WAIT);

    uint32_t dir_mask = (1u << PIN_SELECT) | (1u << PIN_A0) | (1u << PIN_A1) |
                        (1u << PIN_INP) | (1u << PIN_OUT) | (1u << PIN_WAIT);

    gpio_init_mask(pin_mask);
    gpio_set_dir_in_masked(dir_mask);

    gpio_set_input_enabled(PIN_SELECT, true);
    gpio_set_input_enabled(PIN_A0, true);
    gpio_set_input_enabled(PIN_A1, true);
    gpio_set_input_enabled(PIN_INP, true);
    gpio_set_input_enabled(PIN_OUT, true);

    //gpio_set_irq_callback(altairBusISR);


    gpio_pull_down(PIN_SELECT);
    gpio_pull_down(PIN_A0);
    gpio_pull_down(PIN_A1);
    gpio_pull_down(PIN_INP);
    gpio_pull_down(PIN_OUT);

    log->dbgWrite(stringFormat("init IO: pin_mask 0x%04x, dir_mask 0x%04x\r\n", pin_mask, dir_mask));

    ioInitd = true;
}

bool isIoInitd()
{
    return (ioInitd);
}

void setISREnables(bool state)
{
    gpio_set_irq_enabled_with_callback (PIN_INP, GPIO_IRQ_EDGE_RISE, true, altairBusISR);
    gpio_set_irq_enabled_with_callback (PIN_OUT, GPIO_IRQ_EDGE_RISE, true, altairBusISR);

    if (ioInitd)
    {
    //    gpio_set_irq_enabled(PIN_INP, GPIO_IRQ_EDGE_RISE, state);
    //    gpio_set_irq_enabled(PIN_OUT, GPIO_IRQ_EDGE_RISE, state);
    }
}

bool updateIO()
{
    static uint32_t allPins = 0;

    bool retval = false;

    switch(busEvent)
    {
        case BUS_NONE:
        {

        }  break;

        case BUS_INPUT:
        {
            log->dbgWrite("BUS_INPUT event!\r\n");
        }  break;

        case BUS_OUTPUT:
        {
            log->dbgWrite("BUS_OUTPUT event!\r\n");
        }  break;

        case BUS_NO_SELECT:
        {
            log->dbgWrite("BUS_NO_SELECT event!\r\n");
        }  break;

        case BUS_UNKNOWN:
        {
            log->dbgWrite("BUS_UNKNOWN event!\r\n");
        }  break;
    }

    busEvent = BUS_NONE;

    uint32_t pinsNow = gpio_get_all() & 0x003f;
    if (pinsNow != allPins)
    {
        log->infoWrite(stringFormat("All pins from 0x%04x to 0x%04x\r\n", allPins, pinsNow));
    }

    allPins = pinsNow;

    return (retval);
}
