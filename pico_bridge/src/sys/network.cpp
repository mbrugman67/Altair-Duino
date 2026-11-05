
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/sntp.h"
#include "pico/util/datetime.h"
#include "pico/aon_timer.h"
#include "pico/mutex.h"

#include <stdlib.h>
#include <cstddef>

#include "network.h"
#include "../utils/stringFormat.h"

NET88* NET88::instance = NULL;

auto_init_mutex(aon_timer_mutex);
static bool aon_timer_is_initialised = false;


// callback for lwIP/SNTP to set the aon_timer to UTC
// we configure SNTP to call this function when it receives a valid NTP timestamp
// (see lwipopts.h)
void sntp_set_system_time_us(uint32_t sec, uint32_t us) {
    printf("sntp_set_system_time_us(%d, %d)\r\n", sec, us);

    static struct timespec ntp_ts;
    ntp_ts.tv_sec = sec;
    ntp_ts.tv_nsec = us * 1000;

    if (aon_timer_is_initialised) {
        // wait up to 10ms to obtain exclusive access to the aon_timer
        if (mutex_enter_timeout_ms (&aon_timer_mutex, 10)) {
            aon_timer_set_time(&ntp_ts);
            mutex_exit(&aon_timer_mutex);   // release the mutex as soon as possible
            printf("-> updated system time from NTP\r\n");
        } else {
            printf("-> skipped NTP system time update (aon_timer was busy)\r\n");
        }
    } else {
        // the aon_timer is uninitialised so we don't need exclusive access
        aon_timer_is_initialised = aon_timer_start(&ntp_ts);
        printf("-> initialised system time from NTP\r\n");
    }
}

// callback for lwIP/SNTP to read system time (UTC) from the aon_timer
// we configure SNTP to call this function to read the current UTC system time,
// eg to calculate the roundtrip transmission delay (see lwipopts.h)
void sntp_get_system_time_us(uint32_t *sec_ptr, uint32_t * us_ptr) {
    static struct timespec sys_ts;
    // we don't need exclusive access because we are on the background thread
    aon_timer_get_time(&sys_ts);
    *sec_ptr = sys_ts.tv_sec;
    *us_ptr = sys_ts.tv_nsec / 1000;
}


/********************************************************
 * getInstance
 ********************************************************
 * get the single instance of the class, instantiate
 * if necessary.
 *
 * Note - there is no instance count or deletion - this
 * is embedded, baby; things stay for the duration!
 *******************************************************/
NET88* NET88::getInstance()
{
    // If this is the first call, instantiate it and
    // do the init method
    if (!instance)
    {
        instance = new NET88();
        instance->init();
    }

    // return the single instance
    return (instance);
}

void NET88::init()
{
    // get the singleton instance of the log object
    log = logger::getInstance();

    setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1);

    // Wireless module init, no RTOS
    if (!cyw43_arch_init())
    {
        log->dbgWrite("cyw43_arch_init() done\r\n");
        log->dbgWrite(stringFormat("Attempting connect to '%s' with password '%s'\r\n", WIFI_SSID, WIFI_PASSWORD).c_str());

        // set client mode (connect to a WiFi router, etc.)
        cyw43_arch_enable_sta_mode();

        // try to connect & authenticate
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
        {
            log->dbgWrite("failed to connect\n");
        }
        else
        {
            log->dbgWrite("WiFi Connected!\r\n");
        }
    }
    else
    {
        log->dbgWrite("WiFi module not init'd\r\n");
    }
}

void NET88::doTime()
{
    struct timespec ts;
    struct tm tm;

    if(aon_timer_is_initialised)
    {
        // safely read the current time as UTC seconds and ms since the epoch
        get_time_utc(&ts);

        // if you don't need the date/time fields you can call `ctime()` or one of its variants
        // here to convert the UTC seconds count to a string like "Mon Oct 27 22:06:08 2025\n".
        // If you have defined a valid 'TZ' the string will be in local time, otherwise UTC.
        //printf("%s", ctime(&(ts.tv_sec)));

        // you can extract the date/time fields using `localtime()` or one of its variants. If you
        // have defined a valid 'TZ' then the field values will be in local time, otherwise UTC.
        pico_localtime_r(&(ts.tv_sec), &tm);

        // display the name of the currently active local timeszone, if defined
        if (getenv("TZ")) {
            printf("%s: ", tm.tm_isdst ? tzname[0]: tzname[1]);
            // <time.h> defines `extern char *tzname[2]` to hold the names of the POSIX timezones
        } else {
            printf("UTC: ");
        }

        // you can use `asctime()` and its variants to convert the date/time fields into a string
        // like: "Mon Oct 27 22:06:08 2025\n". If you need more flexibility consider `strftime()`
        printf("%s", asctime(&tm));
    }
    else
    {
        printf("Time not running yet :/");
    }

    printf("\r\n");
}

bool NET88::hasGoodIP()
{
    uint8_t *ip_address = (uint8_t*)&(netif_default->ip_addr.addr);
    uint8_t val = ip_address[0] | ip_address[1] | ip_address[2] | ip_address[3];

    if (val)        return (true);
    else            return (false);
}

std::string NET88::getIPAddress()
{
    uint8_t *ip_address = (uint8_t*)&(netif_default->ip_addr.addr);
    return (stringFormat("%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]));
}

void NET88::initNTP()
{
    // initialise the lwIP/SNTP application
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    ip_addr_t ntp;
    const char pool[] = {144, 202, 62, 209};
    ipaddr_aton(pool, &ntp);
    sntp_setserver(0, &ntp);

    sntp_init();
}


int NET88::get_time_utc(struct timespec *ts_ptr)
{
    int retval = 1;
    if (mutex_enter_timeout_ms(&aon_timer_mutex, 10))
    {
        aon_timer_get_time(ts_ptr);
        mutex_exit(&aon_timer_mutex);
        retval = 0;
    }
    return retval;
}
