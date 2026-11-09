/**********************************************************************
* Core1 main routine for 88-WiFi
***********************************************************************/

// Pico specific includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "../utils/utils.h"
#include "../utils/mlogger.h"

/***************************************
* Main entry point for core 1
***************************************/
void core1_main()
{
    logger* log = logger::getInstance();
    log->infoWrite("Entering core 1 main()\r\n");
    while(true)
    {
        heartbeat();
    }
}
