
#include <register/soc/gpio_struct.h>

// GPIO Pin Definitions
#define USER_LED    21
#define PIN_A0      1
#define PIN_A1      2
#define PIN_SELECT  3
#define PIN_INP     5 
#define PIN_OUT     4
#define PIN_WAIT    43

#define PIN_BLUE    44
#define PIN_PURPLE  7 

static volatile bool input = false;
static volatile bool output = false;
static volatile int a0 = 0;
static volatile int a1 = 0;

static volatile uint32_t blueOut;
static volatile uint32_t purpleOut;
static volatile uint32_t inputs;

static volatile gpio_dev_t *ioRegs;
static volatile gpio_dev_t save;

void ESP_INTR_FLAG_IRAM isrOutputRising()
{
  //if (gpio_get_level((gpio_num_t)PIN_SELECT))
  if ((ioRegs->in >> PIN_SELECT) & 1)
  {
    //inputs = ioRegs->in1.val;
    //memcpy((void*)&save, (void*)ioRegs, sizeof(gpio_dev_t));
    //gpio_set_level((gpio_num_t)PIN_BLUE, 1);
    ioRegs->out1_w1ts.val = blueOut;

    a0 = ((ioRegs->in >> PIN_A0) & 1);
    a1 = ((ioRegs->in >> PIN_A1) & 1);

    output = true;
  }
}
 
void ARDUINO_ISR_ATTR isrInputRising()
{
  //if (gpio_get_level((gpio_num_t)PIN_SELECT))
  if ((ioRegs->in >> PIN_SELECT) & 1)
  {
    //gpio_set_level((gpio_num_t)PIN_PURPLE, 1);
    ioRegs->out_w1ts = purpleOut;


    a0 = ((ioRegs->in >> PIN_A0) & 1);
    a1 = ((ioRegs->in >> PIN_A1) & 1);

    input = true;
  }
}

void setup() 
{
  blueOut = 1 << (0x20 - (0x20 - (44 & 0x1f)));
  purpleOut = 1 << (0x20 - (0x20 - (7 & 0x1f)));
  //ioRegs = (gpio_dev_t*)0x3fc94f70;
  ioRegs = (gpio_dev_t*)0x60004000;
  memset((void*)&save, 0, sizeof(gpio_dev_t));

  pinMode(USER_LED, OUTPUT);

  pinMode(PIN_A0, INPUT);
  pinMode(PIN_A1, INPUT);
  pinMode(PIN_SELECT, INPUT);
  pinMode(PIN_INP, INPUT);
  pinMode(PIN_OUT, INPUT);
  pinMode(PIN_WAIT, INPUT);

  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_PURPLE, OUTPUT);

  Serial.begin();
  delay(2000);

  Serial.printf("Setup done()\r\n");

  attachInterrupt(PIN_OUT, isrOutputRising, RISING);
  attachInterrupt(PIN_INP, isrInputRising, RISING);
}

void loop() 
{
  static uint32_t flashTime = millis();

  if (input)
  {
    digitalWrite(PIN_PURPLE, 0);
    input = false;

    Serial.printf("INPUT event  -> A0-%d, A1-%d\r\n", a0, a1);
  }

  if (output)
  {
    digitalWrite(PIN_BLUE, 0);
    output = false;
    /*
    Serial.printf("OUTPUT event -> A0-%d, A1-%d, inputs 0x%08lx\r\n", a0, a1, inputs);
    Serial.printf("->bt_select   0x%08lx\r\n", save.bt_select);
    Serial.printf("->out         0x%08lx\r\n", save.out);
    Serial.printf("->out_w1ts    0x%08lx\r\n", save.out_w1ts);
    Serial.printf("->out_w1tc    0x%08lx\r\n", save.out_w1tc);
    Serial.printf("->out1        0x%08lx\r\n", save.out1.val);
    Serial.printf("->out1_w1ts   0x%08lx\r\n", save.out1_w1ts.val);
    Serial.printf("->out1_w1tc   0x%08lx\r\n", save.out1_w1tc.val);
    Serial.printf("->sdio_select 0x%08lx\r\n", save.sdio_select.val);
    Serial.printf("->enable      0x%08lx\r\n", save.enable);
    Serial.printf("->enable_w1ts 0x%08lx\r\n", save.enable_w1ts);
    Serial.printf("->enable_w1tc 0x%08lx\r\n", save.enable_w1tc);
    Serial.printf("->enable1     0x%08lx\r\n", save.enable1.val);
    Serial.printf("->enable1w1ts 0x%08lx\r\n", save.enable1_w1ts.val);
    Serial.printf("->enable1w1tc 0x%08lx\r\n", save.enable1_w1tc.val);
    Serial.printf("->strap       0x%08lx\r\n", save.strap.val);
    Serial.printf("->in          0x%08lx\r\n", save.in);
    Serial.printf("->in1         0x%08lx\r\n", save.in1.val);
    */
  }

  if ((millis() - flashTime) > 500)
  {
    flashTime = millis();
  }
}

