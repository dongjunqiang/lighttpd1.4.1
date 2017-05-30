#ifndef _LUNA_MACEO_H_
#define _LUNA_MACEO_H_

#include "luna.h"

#define DEBUG_APPEND(if_debug, log_debug, fmt, ...) \
	if (if_debug) { debug_append(log_debug, fmt, ##__VA_ARGS__); }

// #define _LOG_ERROR_WRITE_ // comment this line to disable LOG_ERROR_WRITE

#ifdef _LOG_ERROR_WRITE_
    #define LOG_ERROR_WRITE(srv, fmt, ...) \
        log_error_write(srv, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define LOG_ERROR_WRITE(srv, fmt, ...)
#endif

#endif

