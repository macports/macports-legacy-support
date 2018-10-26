
#ifndef _MACPORTS_TIME_H_
#define _MACPORTS_TIME_H_

// Include the primary system time.h
#include_next <time.h>

// MP support header
#include "MacportsLegacySupport.h"

// Legacy implementation of clock_gettime
#if __MP_LEGACY_SUPPORT_GETTIME__

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  1 // CALENDAR_CLOCK
#define CLOCK_MONOTONIC 0 // SYSTEM_CLOCK
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int clock_gettime( int clk_id, struct timespec *ts );

#ifdef __cplusplus
}
#endif

#endif // _MP_LEGACY_SUPPORT_GETTIME__

#endif // _MACPORTS_TIME_H_
