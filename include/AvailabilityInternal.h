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

#ifndef _MACPORTS_AVAILABILITY_INTERNAL_H_
#define _MACPORTS_AVAILABILITY_INTERNAL_H_

/*
 * This is a wrapper header for AvailabilityInternal.h, to handle its absence
 * from the 10.4 SDK.  In that case, we provide a substitute; otherwise we
 * just pass through the SDK header.
 *
 * We also provide a dummy definition of __has_include() when the compiler
 * doesn't provide it and we're using a 10.14+ SDK, which uses it here
 * without checking.  It would probably be OK to do this for all SDKs,
 * but for safety we limit it to the relevant cases.  Note that this has
 * to be done *before* the include_next.
 *
 * A similar issue exists with __has_builtin() in the 14.x+ SDK.  In this
 * case, the offending code attempts to handle the missing feature by
 * including a defined() condition, but that doesn't actually work because
 * the intrinsic needs to be parseable before evaluating the boolean.
 * So we again provide a default when needed.
 *
 * For both of the above, we only temporarily provide the dummy definitions
 * during the include_next, to avoid confusing other includes, such as the
 * __has_include() mess in MacTypes.h.  We use header-specific flag macros
 * for this, to avoid nesting issues.
 *
 * There'a complication in the case where AvailabilityMacros.h (included by
 * sdkversion.h) #includes Availability.h (currently in 10.9+ SDKs).  In that
 * case, the sdkversion.h #include below does nothing due to its guard
 * macro (avoiding infinite recursion), but then we're here before defining
 * __MPLS_SDK_MAJOR.  Since we know that a 10.4 AvailabilityMacros.h would
 * never include its nonexistent Availability.h, it's safe to apply the
 * non-10.4 treatment in that case.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#ifdef __MPLS_SDK_MAJOR

#if __MPLS_SDK_MAJOR >= 101400 && !defined(__has_include)
#define __has_include(x) 0
#define __MPLS_DUMMY_HAS_INCLUDE_AVAIL_INT
#endif

#if __MPLS_SDK_MAJOR >= 140000 && !defined(__has_builtin)
#define __has_builtin(x) 0
#define __MPLS_DUMMY_HAS_BUILTIN_AVAIL_INT
#endif

#endif /* __MPLS_SDK_MAJOR */

#if defined(__MPLS_SDK_MAJOR) && __MPLS_SDK_MAJOR < 1050
#include <_macports_extras/tiger_only/AvailabilityInternal.h>
#else
#include_next <AvailabilityInternal.h>
#endif

#ifdef __MPLS_DUMMY_HAS_INCLUDE_AVAIL_INT
#undef __has_include
#undef __MPLS_DUMMY_HAS_INCLUDE_AVAIL_INT
#endif

#ifdef __MPLS_DUMMY_HAS_BUILTIN_AVAIL_INT
#undef __has_include
#undef __MPLS_DUMMY_HAS_BUILTIN_AVAIL_INT
#endif

#endif /* _MACPORTS_AVAILABILITY_INTERNAL_H_ */
