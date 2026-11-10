
#include "hardware/gpio.h"
#include "hardware/i2c.h"

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
static volatile uint32_t isrInputState = 0;

void __time_critical_func(altairBusISR)(uint gpio, uint32_t event_mask)
{
    isrInputState = gpio_get_all() & 0x3f;

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

    gpio_init_mask(pin_mask);

    gpio_set_dir(PIN_A0, GPIO_IN);
    gpio_set_dir(PIN_A1, GPIO_IN);
    gpio_set_dir(PIN_SELECT, GPIO_IN);
    gpio_set_dir(PIN_INP, GPIO_IN);
    gpio_set_dir(PIN_OUT, GPIO_IN);
    gpio_set_dir(PIN_WAIT, GPIO_IN);

    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PIN_I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C0_SCL);
    gpio_pull_up(PIN_I2C0_SDA);


    log->dbgWrite(stringFormat("init IO: pin_mask 0x%04x\r\n", pin_mask));

    ioInitd = true;
}

bool isIoInitd()
{
    return (ioInitd);
}

void setISREnables(bool state)
{
    gpio_set_irq_enabled_with_callback (PIN_SELECT, GPIO_IRQ_EDGE_RISE, true, altairBusISR);
    //gpio_set_irq_enabled_with_callback (PIN_OUT, GPIO_IRQ_EDGE_RISE, true, altairBusISR);

    if (ioInitd)
    {
    //    gpio_set_irq_enabled(PIN_INP, GPIO_IRQ_EDGE_RISE, state);
    //    gpio_set_irq_enabled(PIN_OUT, GPIO_IRQ_EDGE_RISE, state);
    }
}

bool updateIO()
{
    bool retval = false;

    switch(busEvent)
    {
        case BUS_NONE:
        {

        }  break;

        case BUS_INPUT:
        {
            log->dbgWrite(stringFormat("BUS_INPUT event!  Pins = 0x%04x\r\n", isrInputState));
        }  break;

        case BUS_OUTPUT:
        {
            log->dbgWrite(stringFormat("BUS_OUTPUT event!  Pins = 0x%04x\r\n", isrInputState));
        }  break;

        case BUS_NO_SELECT:
        {
            //log->dbgWrite("BUS_NO_SELECT event!\r\n");
        }  break;

        case BUS_UNKNOWN:
        {
            log->dbgWrite(stringFormat("BUS_UNKNOWN event!  Pins = 0x%04x\r\n", isrInputState));
        }  break;
    }

    busEvent = BUS_NONE;

    return (retval);
}
