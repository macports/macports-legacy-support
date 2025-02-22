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

#ifndef _MACPORTS_MACH_TIME_H_
#define _MACPORTS_MACH_TIME_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system mach/mach_time.h */
#include_next <mach/mach_time.h>

__MP__BEGIN_DECLS

#if __MPLS_SDK_SUPPORT_APPROX_TIME__

uint64_t mach_approximate_time(void);

#endif /* __MPLS_SDK_SUPPORT_APPROX_TIME__ */

#if __MPLS_SDK_SUPPORT_CONTINUOUS_TIME__

uint64_t mach_continuous_time(void);
uint64_t mach_continuous_approximate_time(void);

#endif /* __MPLS_SDK_SUPPORT_CONTINUOUS_TIME__ */

__MP__END_DECLS

#endif /* _MACPORTS_MACH_TIME_H_ */
