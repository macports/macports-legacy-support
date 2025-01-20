/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_GETTIME__

#include <errno.h>
#include <stddef.h>
#include <time.h>

#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/resource.h>

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_time.h>
#include <mach/thread_act.h>

#define BILLION32 1000000000U
#define BILLION64 1000000000ULL

/* Interim implementation of clock_gettime_nsec_np() as wrapper */
uint64_t
clock_gettime_nsec_np(clockid_t clk_id)
{
  struct timespec ts;
  if (clock_gettime(clk_id, &ts)) return 0;
  return ts.tv_sec * BILLION64 + ts.tv_nsec;
}

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
      struct timeval boottime;
      size_t boottime_len = sizeof(boottime);
      int bt_mib[] = {CTL_KERN, KERN_BOOTTIME};
      size_t bt_miblen = sizeof(bt_mib) / sizeof(bt_mib[0]);
      ret = sysctl(bt_mib, bt_miblen, &boottime, &boottime_len, NULL, 0);
      if (ret != KERN_SUCCESS) { return ret; }
      struct timeval tv;
      ret = gettimeofday(&tv, NULL);
      timersub(&tv, &boottime, &tv);
      ts->tv_sec  = tv.tv_sec;
      ts->tv_nsec = tv.tv_usec * 1000;
      ret = 0;
    }
    else if ( CLOCK_PROCESS_CPUTIME_ID == clk_id )
    {
      struct rusage ru;
      ret = getrusage(RUSAGE_SELF, &ru);
      timeradd(&ru.ru_utime, &ru.ru_stime, &ru.ru_utime);
      ts->tv_sec  = ru.ru_utime.tv_sec;
      ts->tv_nsec = ru.ru_utime.tv_usec * 1000;
    }
    else if ( CLOCK_THREAD_CPUTIME_ID == clk_id )
    {
      mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
      thread_basic_info_data_t info;

      thread_port_t thread = mach_thread_self();
      ret = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);
      mach_port_deallocate(mach_task_self(), thread);

      time_value_add(&info.user_time, &info.system_time);
      ts->tv_sec  = info.user_time.seconds;
      ts->tv_nsec = info.user_time.microseconds * 1000;
    }
    else if ( CLOCK_MONOTONIC_RAW == clk_id ||
              CLOCK_MONOTONIC_RAW_APPROX == clk_id ||
              CLOCK_UPTIME_RAW == clk_id ||
              CLOCK_UPTIME_RAW_APPROX == clk_id )
    {
      static mach_timebase_info_data_t timebase;
      if ( 0 == timebase.numer || 0 == timebase.denom ) {
        const kern_return_t kr = mach_timebase_info( &timebase );
        if ( kr != KERN_SUCCESS ) { return kr; }
      }
      uint64_t tdiff =  mach_absolute_time() * ( timebase.numer / timebase.denom );
      ts->tv_sec  = tdiff / BILLION32;
      ts->tv_nsec = tdiff % BILLION32;
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
         CLOCK_MONOTONIC == clk_id ||
         CLOCK_PROCESS_CPUTIME_ID == clk_id ||
         CLOCK_THREAD_CPUTIME_ID == clk_id)
    {
      // return 1us precision
      ts->tv_sec  = 0;
      ts->tv_nsec = 1000;
      ret         = 0;
    }
    else if ( CLOCK_MONOTONIC_RAW == clk_id ||
              CLOCK_MONOTONIC_RAW_APPROX == clk_id ||
              CLOCK_UPTIME_RAW == clk_id ||
              CLOCK_UPTIME_RAW_APPROX == clk_id )
    {
      // return 1ns precision
      ts->tv_sec  = 0;
      ts->tv_nsec = 1;
      ret         = 0;
    }
  }
  return ret;
}

int
clock_settime(clockid_t clk_id, const struct timespec *ts)
{
  struct timeval tv;

  switch (clk_id) {

  case CLOCK_REALTIME:
    tv.tv_sec = ts->tv_sec;
    tv.tv_usec = ts->tv_nsec / 1000;
    return settimeofday(&tv, NULL);

  default:
    errno = EINVAL;
    return -1;
  }
}

#endif /* __MPLS_LIB_SUPPORT_GETTIME__ */

#if __MPLS_LIB_SUPPORT_TIMESPEC_GET__

#include <time.h>

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

#endif /* __MPLS_LIB_SUPPORT_TIMESPEC_GET__ */
