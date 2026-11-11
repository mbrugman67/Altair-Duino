
#ifndef GLOBAL_H_
#define GLOBAL_H_

#define CORE_0_WATCHDOG_TIME    666
#define CORE_1_WATCHDOG_TIME    334
#define WATCHDOG_TIMEOUT        10000

#define PIN_SELECT              2
#define PIN_A0                  0
#define PIN_A1                  1
#define PIN_INP                 3
#define PIN_OUT                 4
#define PIN_WAIT                5
#define PIN_I2C0_SCL           13
#define PIN_I2C0_SDA           12

#define BIT(x)          (1u << x)
#define GETBIT(w,b)     (w & BIT(b))
#define SETBIT(w,b)     (w |= BIT(b))
#define CLRBIT(w,b)     (w &= ~(BIT(b)))

extern uint32_t dbgCount;
extern volatile uint32_t isrCount;
extern volatile uint32_t isrFail;

#endif // GLOBAL_H_
