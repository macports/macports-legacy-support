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

/*
 * This header exists solely in the 10.5 SDK, and is approximately equivalent
 * to AvailabilityMacros.h, except for the leading underscores in its macros.
 * It doesn't play nicely with our "older SDK" hack in sdkversions.h, so
 * we tweak it a bit here.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Now fix up the values we fudged */

#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
#undef __MAC_OS_X_VERSION_MIN_REQUIRED
#define __MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_MIN_REQUIRED
#endif

#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#undef __MAC_OS_X_VERSION_MAX_ALLOWED
#define __MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_MAX_ALLOWED
#endif

#include_next <available.h>
