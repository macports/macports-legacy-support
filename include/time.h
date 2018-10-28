
/*
 * Copyright (c) 2010 Chris Jones <jonesc@macports.org>
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
