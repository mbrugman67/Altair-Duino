
#ifndef UTILS_H_
#define UTILS_H_

#include <string>

std::string stringFormat(const std::string& fmt, ...);

void heartbeat();
void chkForReset();

#endif // UTILS_H_
