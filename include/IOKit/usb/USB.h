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

/*
 * This header and some of its children mistakenly use #if KERNEL,
 * #if __cplusplus, and #if __OPEN_SOURCE__ when they should be using #ifdef.
 * This results in some "undefined" warnings.  So we try to disable that
 * warning around the include_next, but "diagnostic push/pop" is unavailable
 * in early GCCs, where we're stuck with it.
 *
 * In addition, the 26+ SDKs use "#pragma clang deprecated", which is not
 * supported by some earlier clangs, leading to a warning, so we disable
 * that warning as well with those SDKs.
 *
 * In addition, all versions define kUSBLowLatencyIsochTransferKey as a
 * multi-character literal, which provokes a warning from GCC 4.9+, so
 * we disable that for GCC as well.
 */

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wundef"
  #if __MPLS_SDK_MAJOR >= 260000
    #pragma clang diagnostic ignored "-Wunknown-pragmas"
  #endif
#elif defined(__GNUC__) \
      && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wundef"
  #pragma GCC diagnostic ignored "-Wmultichar"
#endif

/* Include the primary system IOKit/usb/USB.h */
#include_next <IOKit/usb/USB.h>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 5
  #pragma GCC diagnostic pop
#endif
