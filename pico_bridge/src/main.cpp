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
#include "./utils/utils.h"
#include "./utils/mlogger.h"
#include "./core1/core1.h"

int main()
{
    static logger* log = logger::getInstance();

    // init STDIN/STDOUT through USB
    stdio_init_all();

    sleep_ms(2000);

    // Wireless module init, no RTOS
    if (!cyw43_arch_init())
    {
        printf("cyw43_arch_init() done\r\n");

        printf("WIFI_SSID = %s, WIFI_PASSWORD = %s\r\n", WIFI_SSID, WIFI_PASSWORD);

        cyw43_arch_enable_sta_mode();
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
        {
            printf("failed to connect\n");
        }
        else
        {
            printf("Connected!\r\n");
        }
    }
    else
    {
        printf("wifi module not init'd\r\n");
    }

    // startup core 1
    multicore_launch_core1(core1_main);

    printf("%s", log->pop().c_str());

    while(true)
    {
        chkForReset();
        heartbeat();

        printf("%s", log->pop().c_str());
    }
}
