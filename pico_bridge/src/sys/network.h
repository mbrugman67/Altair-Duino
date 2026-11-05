
#ifndef NETWORK_H_
#define NETOWRK_H_

#include <cstring>

#include "../utils/mlogger.h"

class NET88
{
public:
    // singleton class
    static NET88* getInstance();

    bool hasGoodIP();
    std::string getIPAddress();
    void initNTP();
    void doTime();

protected:

private:
    logger* log;

    void init();
    int get_time_utc(struct timespec *ts_ptr);


    static NET88* instance;
};

#endif //NETWORK_H_
