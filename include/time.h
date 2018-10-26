
#ifndef _MACPORTS_TIME_H_
#define _MACPORTS_TIME_H_

// Include the primary system time.h
#include_next <time.h>

// Implementation of clock_gettime for OSX10.11 and older.
#define __MP_LEGACY_SUPPORT_GETTIME__ __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200
#if __MP_LEGACY_SUPPORT_GETTIME__
static int clock_gettime( int clk_id, struct timespec *ts );
#endif

#endif // _MACPORTS_TIME_H_
