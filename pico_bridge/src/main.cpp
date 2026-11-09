/**********************************************************************
* Main entry point for 88-WiFi bridge project
*
* (C) 2026 M.Brugman
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
***********************************************************************/

// Pico specific includes
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"

// Normal C/C++ includes
#include <cstdio>

// Project includes
#include "./sys/network.h"
#include "./utils/utils.h"
#include "./utils/mlogger.h"
#include "./core1/core1.h"

int main()
{
    // init STDIN/STDOUT through USB
    stdio_init_all();

    logger* log = logger::getInstance();
    NET88* network = NET88::getInstance();

    // short sleep to allow terminal to reconnect
    sleep_ms(2000);
    log->dbgWrite("stdio_init_all() done\r\n");


    // startup core 1
    multicore_launch_core1(core1_main);
    log->dbgWrite("multicore_launch_core1() done\r\n");

    log->infoWrite("\r\n\r\n****************************************************\r\n"
                   "                 Starting main loop"
                   "\r\n****************************************************\r\n\r\n");

    log->infoWrite(stringFormat("Current IP Address => %s\r\n", network->getIPAddress().c_str()));

    while(true)
    {
        chkForReset();
        heartbeat();

        printf("%s", log->pop().c_str());
    }
}
