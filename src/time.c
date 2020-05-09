
/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "time.h"

#if __MP_LEGACY_SUPPORT_GETTIME__

#include <sys/time.h>
#include <mach/mach_time.h>

int clock_gettime( clockid_t clk_id, struct timespec *ts )
{
  int ret = -1;
  if ( ts )
  {
    if      ( CLOCK_REALTIME == clk_id )
    {
      struct timeval tv;
      ret = gettimeofday(&tv, NULL);
      ts->tv_sec  = tv.tv_sec;
      ts->tv_nsec = tv.tv_usec * 1000;
    }
    else if ( CLOCK_MONOTONIC == clk_id )
    {
      const uint64_t t = mach_absolute_time();
      mach_timebase_info_data_t timebase;
      mach_timebase_info(&timebase);
      const uint64_t tdiff = t * timebase.numer / timebase.denom;
      ts->tv_sec  = tdiff / 1000000000;
      ts->tv_nsec = tdiff % 1000000000;
      ret = 0;
    }
  }
  return ret;
}

int clock_getres ( clockid_t clk_id, struct timespec *ts )
{
  int ret = -1;
  if ( ts )
  {
    if ( CLOCK_REALTIME  == clk_id ||
         CLOCK_MONOTONIC == clk_id )
    {
      // return 1us precision
      ts->tv_sec  = 0;
      ts->tv_nsec = 1000;
      ret         = 0;
    }
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
