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

#ifndef _MACPORTS_TIME_H_
#define _MACPORTS_TIME_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system time.h */
#include_next <time.h>

/* The following functions are implemented by Tiger, but the declarations are
 * missing if _ANSI_SOURCE or _POSIX_C_SOURCE are defined, which occurs when
 * _XOPEN_SOURCE is set. */
#if __MPLS_SDK_SUPPORT_TIME_THREAD_SAFE_FUNCTIONS__

__MP__BEGIN_DECLS
#if defined(_ANSI_SOURCE) || defined(_POSIX_C_SOURCE)
char *asctime_r(const struct tm *, char *);
char *ctime_r(const time_t *, char *);
struct tm *gmtime_r(const time_t *, struct tm *);
struct tm *localtime_r(const time_t *, struct tm *);
#endif /* defined(_ANSI_SOURCE) || defined(_POSIX_C_SOURCE) */
__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_TIME_THREAD_SAFE_FUNCTIONS__ */

/* Legacy implementation of clock_gettime */
#if __MPLS_SDK_SUPPORT_GETTIME__

/*
 * Define clock_id type and values as in the 10.12 SDK, but without the
 * pointless __CLOCK_AVAILABILITY.
 */
typedef enum {
_CLOCK_REALTIME = 0,
#define CLOCK_REALTIME _CLOCK_REALTIME
_CLOCK_MONOTONIC = 6,
#define CLOCK_MONOTONIC _CLOCK_MONOTONIC
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
_CLOCK_MONOTONIC_RAW = 4,
#define CLOCK_MONOTONIC_RAW _CLOCK_MONOTONIC_RAW
_CLOCK_MONOTONIC_RAW_APPROX = 5,
#define CLOCK_MONOTONIC_RAW_APPROX _CLOCK_MONOTONIC_RAW_APPROX
_CLOCK_UPTIME_RAW = 8,
#define CLOCK_UPTIME_RAW _CLOCK_UPTIME_RAW
_CLOCK_UPTIME_RAW_APPROX = 9,
#define CLOCK_UPTIME_RAW_APPROX _CLOCK_UPTIME_RAW_APPROX
#endif
_CLOCK_PROCESS_CPUTIME_ID = 12,
#define CLOCK_PROCESS_CPUTIME_ID _CLOCK_PROCESS_CPUTIME_ID
_CLOCK_THREAD_CPUTIME_ID = 16
#define CLOCK_THREAD_CPUTIME_ID _CLOCK_THREAD_CPUTIME_ID
} clockid_t;

__MP__BEGIN_DECLS

extern int clock_gettime( clockid_t clk_id, struct timespec *ts );
extern int clock_getres ( clockid_t clk_id, struct timespec *ts );

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_GETTIME__ */

/* Legacy implementation of timespec */
#if __MPLS_SDK_SUPPORT_TIMESPEC_GET__

#ifndef TIME_UTC
#define TIME_UTC	1	/* time elapsed since epoch */
#endif

__MP__BEGIN_DECLS

extern int timespec_get(struct timespec *ts, int base);

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_TIMESPEC_GET__ */

#endif /* _MACPORTS_TIME_H_ */
