#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include "syslog.h"

using namespace std;

enum LOG_LEVEL {
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL
};

void InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message);
void ExitLog();

#endif