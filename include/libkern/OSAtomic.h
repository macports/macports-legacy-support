/*
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

#ifndef _MACPORTS_OSATOMIC_H_
#define _MACPORTS_OSATOMIC_H_

/*
 * This is a wrapper to allow building OSAtomic.h in C89 mode.  The only issue
 * is the lack of 'inline'.  The SDK version switched to '__inline' as of
 * the 10.6 SDK.  We mimic that with a temporary macro definition.
 */

#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) \
    && !defined(inline)

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1060

#define inline __inline
#define __MPLS_TEMP_INLINE

#endif /* Not <10.6 */

#endif /* Not < C99 */

/* Include the primary system libkern/OSAtomic.h */
#include_next <libkern/OSAtomic.h>

#ifdef __MPLS_TEMP_INLINE
#undef inline
#undef __MPLS_TEMP_INLINE
#endif

#endif /* _MACPORTS_OSATOMIC_H_ */
