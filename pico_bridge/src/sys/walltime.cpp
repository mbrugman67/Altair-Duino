/********************************************************
 * walltime.cpp
 ********************************************************
 * handle real-time clock management; setting, reading,
 * timezone
 *
 * NOTE: The assumption here is that the wifi module
 * is already set up and running.  This just sends
 * and receives a UDP packet to do the time thing
 *
 * 8 April 2021, M.Brugman
 *
 *******************************************************/

#include <cstring>
#include <machine/endian.h>

#include "pico/stdlib.h"
#include "walltime.h"
#include "../utils/utils.h"
#include "network.h"

static NET88* network = NET88::getInstance();

// Load some timezones.  If yours isn't hear, add it now!
walltime::zdata_t walltime::zones[] = {
                   {"ADST9AKDT",    "US Alaska"},
                   {"PST8PDT",      "US Pacific"},
                   {"PST8",         "US Pacific (no DST)"},
                   {"MST7MDT",      "US Mountain"},
                   {"MST7",         "US Mountain (no DST)"},
                   {"CST6CDT",      "US Central"},
                   {"CST6",         "US Central (no DST)"},
                   {"EST5EDT",      "US Eastern"},
                   {"EST5",         "US Easter (no DST)"},
                   {"GMT-",         "GMT (Zulu)"},
                   {"BST-1",        "British Standard"},
                   {"CET-1CEST",    "Central European"}
                   };

/********************************************************
 * Constructor
 ********************************************************
 * Set the IP address and init the RTC module.
 *
 * Future version should use DNS to hit pool.ntp.org
 * to get a rotation of timeservers instead of hard-
 * coding it.
 *******************************************************/
walltime::walltime()
{
    //timeServer.fromString("129.6.15.28");
    timeValid = false;
    //rtc_init();
}

/********************************************************
 * listZones()
 ********************************************************
 * return a std string of all possible timezones.
 *******************************************************/
const std::string walltime::listZones()
{
    char out[sizeof(zones) + 64] = {0};
    size_t remain = sizeof(out);
    size_t count = sizeof(zones) / sizeof(zones[0]);

    for (size_t ii = 0; ii < count; ++ii)
    {
        char line[sizeof(zones[0]) + 4];
        snprintf(line, sizeof(zones[0]) + 2, "%s: %s\n", zones[ii].name, zones[ii].desc);
        strncat(out, line, remain);
        remain -= strlen(line);
    }

    return (std::string(out));
}

/********************************************************
 * setTimezone()
 ********************************************************
 * Set the timezone, based on one of the existing zones
 * in the list
 *******************************************************/
bool walltime::setTimezone(const std::string& zoneName)
{
    // Set the environment variable for the timezone
    // let gcc and the standard library do all the real
    // work for use
    setenv("TZ", zoneName.c_str(), 1);
    return (true);
}

/********************************************************
 * timeString()
 ********************************************************
 * return a human-readable time string
 *******************************************************/
std::string walltime::timeString()
{
    std::string ret = std::string("<not set>");

    struct tm tm;
    if (network->getTime(tm))
    {
        ret = stringFormat("%02d:%02d:%02d",
            tm.tm_hour, tm.tm_min, tm.tm_sec);

        return (ret);
    }

    return (ret);
}

/********************************************************
 * dateString()
 ********************************************************
 * return a human-readable date string
 *******************************************************/
std::string walltime::dateString()
{
    std::string ret = std::string("<not set>");

    struct tm tm;
    if (network->getTime(tm))
    {
        ret = stringFormat("%d/%d/%d",
            tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);

        return (ret);
    }

    return (ret);
}


/********************************************************
 * logTimeString()
 ********************************************************
 * return a human-readable date string formatted for logging
 *******************************************************/
std::string walltime::logTimeString()
{
    std::string ret = std::string("<not set>");

    struct tm tm;
    if (network->getTime(tm))
    {
        ret = stringFormat("%04d%02d%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

        return (ret);
    }

    return (ret);
}
