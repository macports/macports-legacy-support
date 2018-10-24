
#ifndef _MACPORTS_TIME_H_
#define _MACPORTS_TIME_H_

// Include the primary system time.h
#include_next <time.h>

// Implementation of clock_gettime for OSX10.11 and older.
#if __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200
#include <mach/clock.h>
#include <mach/mach.h>
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  CALENDAR_CLOCK
#define CLOCK_MONOTONIC SYSTEM_CLOCK
#endif
static int
clock_gettime( int clk_id, struct timespec *ts )
{
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service( mach_host_self(), clk_id, &cclock );
  const int ret = clock_get_time( cclock, &mts );
  mach_port_deallocate( mach_task_self(), cclock );
  if ( ts )
  {
    ts->tv_sec  = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
  }
  return ret;
}
#endif

#endif // _MACPORTS_TIME_H_
