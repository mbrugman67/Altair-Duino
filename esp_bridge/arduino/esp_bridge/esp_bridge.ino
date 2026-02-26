#include <Arduino.h>

#include <SoftwareSerial.h>
#include "telnetBridge.h"
#include "wifimgr.h"
#include "menu.h"


EspSoftwareSerial::UART sserial;
WIFIMGR wifiMgr;
MENU serialMenu;
TelnetBridge tn;

void setup() 
{
  Serial.begin();
  sserial.begin(300, SWSERIAL_8N1, 7, 44, false);
  tn.begin();
  serialMenu.begin();
}

void loop() 
{ 
  if (!tn.update(false))
  {
    serialMenu.update();
  }
}
