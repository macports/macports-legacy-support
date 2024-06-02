/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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

#ifndef _MACPORTS_SYS_CDEFS_H_
#define _MACPORTS_SYS_CDEFS_H_

/*
 * This provides definitions for the __DARWIN_C_* macros for earlier SDKs
 * that don't provide them.  Since the definitions are based on #ifndef,
 * there's no need for explicit SDK version thresholds.
 *
 * Note that all SDKs provide adjustments for _POSIX_C_SOURCE where needed.
 */

/* Include the primary system sys/cdefs.h */
#include_next <sys/cdefs.h>

/* The following is copied from the 10.7 SDK, but with additional #ifndefs */

/*
 * Set a single macro which will always be defined and can be used to determine
 * the appropriate namespace.  For POSIX, these values will correspond to
 * _POSIX_C_SOURCE value.  Currently there are two additional levels corresponding
 * to ANSI (_ANSI_SOURCE) and Darwin extensions (_DARWIN_C_SOURCE)
 */
#ifndef __DARWIN_C_ANSI
#define __DARWIN_C_ANSI         010000L
#endif

#ifndef __DARWIN_C_FULL
#define __DARWIN_C_FULL         900000L
#endif

#ifndef __DARWIN_C_LEVEL

#if   defined(_ANSI_SOURCE)
#define __DARWIN_C_LEVEL        __DARWIN_C_ANSI
#elif defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE) && !defined(_NONSTD_SOURCE)
#define __DARWIN_C_LEVEL        _POSIX_C_SOURCE
#else
#define __DARWIN_C_LEVEL        __DARWIN_C_FULL
#endif

#endif /* __DARWIN_C_LEVEL undef */

#endif /* _MACPORTS_SYS_CDEFS_H_ */
