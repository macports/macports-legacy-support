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
 * This is a wrapper header for Availability.h, to handle its absence in
 * the 10.4 SDK.  In that case, we provide a substitute; otherwise we
 * just pass through the SDK header.
 *
 * We don't use a guard macro, since it's not only unnecessary, but actually
 * harmful in this context.  It's unnecessary since all we do here is include
 * other headers which have their own guard macros, and we don't define
 * anything here.  It would be harmful because we need to short-circuit the
 * possible sdkversion.h->AvailabilityMacros.h->Availability.h path where
 * __MPLS_SDK_MAJOR is not yet defined, and hence need a second bite of
 * the apple to get Availability.h for real.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* If __MPLS_SDK_MAJOR isn't defined here, do nothing. */
#ifdef __MPLS_SDK_MAJOR

#if __MPLS_SDK_MAJOR < 1050
#include <_macports_extras/tiger_only/Availability.h>
#else
#include_next <Availability.h>
#endif

#endif /* __MPLS_SDK_MAJOR defined */
