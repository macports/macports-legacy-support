/*
 * Copyright (c) 2021 Evan Miller <emmiller@gmail.com>
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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
 * pack(1) and align=reset don't mix in some versions of GCC.
 * See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=50909
 *
 * The basic problem is that since pack(1) is NOP alignment,
 * GCC 4.3+ ignores it completely, including failing to push an entry
 * onto the alignment stack.  This causes a "corresponding" align=reset
 * to malfunction or fail.  This bug has still not been fixed as of GCC 15.
 *
 * The workaround is to preload the alignment stack with enough
 * "NOP but still pushed" alignments to compensate for the failing
 * pack(1)/reset pairs in the system header.  The number required depends
 * on the SDK version.
 *
 * Although extra dummy pushes are mostly harmless, they may obfuscate
 * actual overpops in the ensuing code, and since we know the SDK version,
 * we use the correct number, including none at all in the SDK 10.15+ cases
 * where pack(1)/reset isn't used.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 101500 \
    && defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) \
    && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
/* 10.4 SDK and up need at least three invocations */
#pragma options align=power
#pragma options align=power
#pragma options align=power
/* 10.8 SDK and up need two more */
#if __MPLS_SDK_MAJOR >= 1080
#pragma options align=power
#pragma options align=power
#endif  /* SDK >= 10.8 */
#endif  /* SDK < 10.15 and bad GCC */

/* Include the primary system IOKit/usb/USB.h */
#include_next <IOKit/usb/USB.h>
