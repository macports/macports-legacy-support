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
 * A complication here is that some recent compilers treat preprocessor
 * undefined warnings as errors, and 10.15+ SDKs reference the possibly
 * undefined TARGET_OS_* macros.  To get around that, we temporarily
 * define TARGET_OS_OSX as 1 if necessary, and then undo that afterward.
 * This assumes that we're actually building for macOS, which should usually
 * be the case with these headers.  Setting TARGET_OS_OSX avoids the check
 * for TARGET_OS_MACCATALYST.  The only possibly unwanted effect of this
 * is the "#define __IPHONE_COMPAT_VERSION  __IPHONE_NA", which is most
 * likely correct if it matters at all.
 *
 * We don't bother with a guard macro here, since repeating the extra wrapper
 * code is a NOP, anyway.
 */

/* Avoid possible error from TARGET_OS_OSX test */
#ifndef TARGET_OS_OSX
#define TARGET_OS_OSX 1
#define __MPLS_TARGET_OS_OSX_UNDEF
#endif

#include_next <AvailabilityMacros.h>

/* Now restore the original TARGET_OS_OSX. */
#ifdef __MPLS_TARGET_OS_OSX_UNDEF
#undef TARGET_OS_OSX
#undef __MPLS_TARGET_OS_OSX_UNDEF
#endif
