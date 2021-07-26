#ifndef PRIVATEBET_LOGGING_H_
#define PRIVATEBET_LOGGING_H_

#include <stdio.h>

static inline char *timenow();

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03

#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

#define BET_PRINT(format, ...)      fprintf(stderr, format, __VA_ARGS__)

#define LOG_FMT             "%s | %-7s | %-15s | %s:%d | "
#define LOG_ARGS(LOG_TAG)   timenow(), LOG_TAG, _FILE, __FUNCTION__, __LINE__

#define NEWLINE     "\n"

#define ERROR_TAG   "ERROR"
#define INFO_TAG    "INFO"
#define DEBUG_TAG   "DEBUG"

#if LOG_LEVEL >= DEBUG_LEVEL
#define BET_LOG_DBG(message, args...)     BET_PRINT(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TAG), ## args)
#else
#define BET_LOG_DBG(message, args...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define BET_LOG_INFO(message, args...)      BET_PRINT(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TAG), ## args)
#else
#define BET_LOG_INFO(message, args...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define BET_LOG_ERR(message, args...)     BET_PRINT(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## args)
#else
#define BET_LOG_ERR(message, args...)
#endif

#if LOG_LEVEL >= NO_LOGS
#define BET_LOG_ERR(condition, message, args...) if (condition) BET_PRINT(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## args)
#else
#define BET_LOG_ERR(condition, message, args...)
#endif

static inline char *timenow() {
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return buffer;
}

#endif // PRIVATEBET_LOGGING_H_
