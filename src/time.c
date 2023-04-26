
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

#define BILLION  1000000000L
#define MILLION  1000000L
#define THOUSAND 1000L

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
    else if ( CLOCK_MONOTONIC == clk_id || CLOCK_MONOTONIC_RAW == clk_id )
    {
      static mach_timebase_info_data_t timebase;
      if ( 0 == timebase.numer || 0 == timebase.denom ) {
        const kern_return_t kr = mach_timebase_info( &timebase );
        if ( kr != KERN_SUCCESS ) { return kr; }
      }
      uint64_t tdiff =  mach_absolute_time() * ( timebase.numer / timebase.denom );
      if ( CLOCK_MONOTONIC == clk_id ) {
        tdiff = THOUSAND * ( tdiff / THOUSAND );
      }
      ts->tv_sec  = tdiff / BILLION;
      ts->tv_nsec = tdiff % BILLION;
      ret = 0;
    }
  }
  return ret;
}

int clock_getres( clockid_t clk_id, struct timespec *ts )
{
  int ret = -1;
  if ( ts )
  {
    if ( CLOCK_REALTIME  == clk_id ||
         CLOCK_MONOTONIC == clk_id )
    {
      // return 1us precision
      ts->tv_sec  = 0;
      ts->tv_nsec = THOUSAND;
      ret         = 0;
    }
    else if ( CLOCK_MONOTONIC_RAW == clk_id )
    {
      // return 1ns precision
      ts->tv_sec  = 0;
      ts->tv_nsec = 1;
      ret         = 0;
    }
  }
  return ret;
}

#endif

#if __MP_LEGACY_SUPPORT_TIMESPEC_GET__

int timespec_get(struct timespec *ts, int base)
{
  switch (base) {
    case TIME_UTC:
      if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
        return 0;
      }
      return base;

    default:
      return 0;
  }
}

#endif
