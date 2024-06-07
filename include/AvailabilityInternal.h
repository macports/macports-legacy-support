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

/*
 * This is a wrapper header for AvailabilityInternal.h, to handle its absence
 * from the 10.4 SDK.  In that case, we provide a substitute; otherwise we
 * just pass through the SDK header.
 *
 * We don't bother with a guard macro, since the included headers will
 * handle that.
 *
 * We also provide a dummy definition of __has_include() when the compiler
 * doesn't provide it and we're using a 10.14+ SDK, which uses it here
 * without checking.  It would probably be OK to do this for all SDKs,
 * but for safety we limit it to the relevant cases.  Note that this has
 * to be done *before* the include_next.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if !__MPLS_PRE_10_14_SDK && !defined(__has_include)
#define __has_include(x) 0
#endif

#if __MPLS_PRE_10_5_SDK
#include <_macports_extras/tiger_only/AvailabilityInternal.h>
#else
#include_next <AvailabilityInternal.h>
#endif
