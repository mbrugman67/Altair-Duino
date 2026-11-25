
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

static uint32_t isrMask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;

static logger* log = logger::getInstance();

static volatile bus_event_t busEvent = BUS_NONE;
static volatile uint32_t isrInputState = 0;

volatile uint32_t dbgCount = 0;
volatile uint32_t isrFail = 0;
volatile uint32_t isrCount = 0;

volatile bool selectHigh = false;
volatile bool inputHigh = false;
volatile bool outputHigh = false;

void __time_critical_func(isrSelectLine)(uint gpio, uint32_t event_mask)
{
    bool up = ((event_mask & GPIO_IRQ_EDGE_RISE) == GPIO_IRQ_EDGE_RISE);
    bool dn = ((event_mask & GPIO_IRQ_EDGE_FALL) == GPIO_IRQ_EDGE_FALL);

    if (up && !dn)
    {
        busEvent = BUS_OUTPUT;
        ++isrCount;
    }
    else if (!up && dn)
    {
        ++dbgCount;
    }
    else if (up && dn)
    {
        ++isrFail;
    }
}

void __time_critical_func(isrInpLine)(uint gpio, uint32_t event_mask)
{
    bool up = ((event_mask & GPIO_IRQ_EDGE_RISE) == GPIO_IRQ_EDGE_RISE);
    bool dn = ((event_mask & GPIO_IRQ_EDGE_FALL) == GPIO_IRQ_EDGE_FALL);

    if (up && !dn)
    {
        busEvent = BUS_OUTPUT;
        ++isrCount;
    }
    else if (!up && dn)
    {
        ++dbgCount;
    }
    else if (up && dn)
    {
        ++isrFail;
    }
}

void __time_critical_func(isrOutLine)(uint gpio, uint32_t event_mask)
{
    if (GETBIT(event_mask, GPIO_IRQ_EDGE_FALL))
    {
        outputHigh = false;
    }
    else if (GETBIT(event_mask, GPIO_IRQ_EDGE_RISE))
    {
        outputHigh = true;
    }
}

/*
void __time_critical_func(altairBusISR)(uint gpio, uint32_t event_mask)
{
    if (GETBIT(event_mask, GPIO_IRQ_EDGE_FALL))
    {
        ++isrFail;
    }

    ++isrCount;

    if (gpio_get(PIN_OUT))
    {
        busEvent = BUS_OUTPUT;
        isrInputState = gpio_get_all() & 0x3f;
        gpio_set_irq_enabled(PIN_SELECT, isrMask, false);
        return;
    }

    if (gpio_get(PIN_INP))
    {
        busEvent = BUS_INPUT;
        isrInputState = gpio_get_all() & 0x3f;
        gpio_set_irq_enabled(PIN_SELECT, isrMask, false);
        return;
    }
}
*/


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
    uint32_t pins[] = {PIN_SELECT, PIN_A0, PIN_A1, PIN_INP, PIN_OUT, PIN_WAIT};
    size_t pinCount = sizeof(pins) / sizeof(pins[0]);

    for (size_t pinNum = 0; pinNum < pinCount; ++pinNum)
    {
        gpio_init(pins[pinNum]);
        gpio_set_slew_rate(pins[pinNum], GPIO_SLEW_RATE_FAST);
        gpio_set_input_hysteresis_enabled(pins[pinNum], false);
        gpio_set_dir(pins[pinNum], GPIO_IN);
    }

    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PIN_I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C0_SCL);
    gpio_pull_up(PIN_I2C0_SDA);

    //gpio_set_irq_callback(&altairBusISR);
    gpio_set_irq_enabled_with_callback(PIN_SELECT, isrMask, true, isrSelectLine);
    //gpio_set_irq_enabled_with_callback(PIN_INP, isrMask, true, isrInpLine);
    //gpio_set_irq_enabled_with_callback(PIN_OUT, isrMask, true, isrOutLine);
}

bool updateIO()
{
    bool retval = false;
    static bool oneShot = false;
    if (!gpio_get(PIN_SELECT))
    {
        oneShot = false;
    }
    else if(!oneShot)
    {
        //dbgCount;
        oneShot = true;
    }

    switch(busEvent)
    {
        case BUS_INPUT:
        {
            log->dbgWrite(stringFormat("BUS_INPUT event!  count= %d\r\n", isrInputState));
            isrInputState = 0;
        }  break;

        case BUS_OUTPUT:
        {
            log->dbgWrite(stringFormat("BUS_OUTPUT event!  count= %d\r\n", isrInputState));
            isrInputState = 0;
        }  break;
    }

    busEvent = BUS_NONE;

    return (retval);
}
