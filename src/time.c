
#include "time.h"

#if __MP_LEGACY_SUPPORT_GETTIME__

#include <sys/time.h>
#include <mach/mach_time.h>

int clock_gettime( int clk_id, struct timespec *ts )
{
  int ret = -1;
  if      ( clk_id == CLOCK_REALTIME )
  {
    struct timeval tv;
    ret = gettimeofday(&tv, NULL);
    ts->tv_sec  = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
  }
  else if ( clk_id == CLOCK_MONOTONIC )
  {
    const uint64_t t = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    const uint64_t tdiff = t * timebase.numer / timebase.denom;
    ts->tv_sec  = tdiff / 1000000000;
    ts->tv_nsec = tdiff % 1000000000;
    ret = 0;
  }
  return ret;
}

/*
#include <mach/clock.h>
#include <mach/mach.h>
int clock_gettime( int clk_id, struct timespec *ts )
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
*/

#endif
