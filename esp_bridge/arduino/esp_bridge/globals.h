#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "wifimgr.h"
#include "menu.h"
#include "telnetBridge.h"

// Global classes
extern EspSoftwareSerial::UART sserial;
extern WIFIMGR wifiMgr;
extern TelnetBridge tn;
extern MENU serialMenu;

#endif // GLOBALS_H_