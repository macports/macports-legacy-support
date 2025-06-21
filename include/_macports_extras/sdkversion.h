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

#ifndef _MACPORTS_SDKVERSION_H_
#define _MACPORTS_SDKVERSION_H_

/*
 * This header provides definitions related to the OS SDK version, for cases
 * where behavior needs to depend on the include tree layout.  The basic
 * method for obtaining the SDK version is to use MAC_OS_X_VERSION_MAX_ALLOWED
 * as defined in AvailabilityMacros.h, which is available in all SDKs.
 *
 * There is, however, a complication in the rare and questionable case where
 * the build is using an SDK version *older* than the build target.  In this
 * case, AvailabilityMacros.h forces ...MAX_ALLOWED to the target OS version
 * (as indicated by MAC_OS_X_VERSION_MIN_REQUIRED), thereby not necessarily
 * correctly reflecting the SDK file contents.
 *
 * To get around this we:
 * 1) Define MAC_OS_X_VERSION_MIN_REQUIRED as the allowable minimum
 * 2) #include <AvailabilityMacros.h>
 * 3) Set up our own flags based on ...MAX_ALLOWED
 * 4) Redefine ...MIN_REQUIRED and ...MAX_ALLOWED with the normal values
 *
 * Note that the fixup for ...MIN_REQUIRED is based on the compiler-supplied
 * __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__, which may be undefined,
 * but only on pre-10.5 systems.  Since we don't support anything older than
 * 10.4, we just assume 10.4 in that case, rather than the more elaborate
 * logic in AvailabilityMacros.h that assumes 10.1 on ppc.
 *
 * Also note that the fake ...MIN_REQUIRED will result in some of the
 * availability macros being defined incorrectly for the actual target OS,
 * but since the purpose of this package is to override Apple's notion of
 * availability, that isn't necessarily inappropriate.  Nevertheless,
 * we make accommodating "earlier" SDKs optional, by using the parameter
 * _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED to specify the earliest "earlier"
 * SDK supported.  If this isn't both defined and earlier than the target OS,
 * then the hack is disabled.
 *
 * There's an additional complication if ...MAX_ALLOWED is already defined
 * initially.  That means that either it was explicitly defined in some
 * fashion, or AvailabilityMacros.h was already included before.  In this
 * case, the above workaround doesn't work, so the best we can do is to take
 * ...MAX_ALLOWED at face value and hope that it works.
 *
 * THUS: If supporting the "older SDK" case is important, this header should
 * be included before anything that might include AvailabilityMacros.h.
 *
 * A further complication is that ...MAX_ALLOWED is normally defined as
 * one of the MAC_OS_X_VERSION... macros, but those definitions may be
 * suppressed in some configurations in 11.x+ SDKs.  In such cases it's
 * impossible to detect the correct SDK version, with or without the
 * "earlier SDK" hack.  To suppress that behavior, we define _DARWIN_C_SOURCE
 * before #including AvailabilityMacros.h, restore its definedness
 * afterward, and add another step:
 *
 * 5) Undefine any version macros whose definition would have been suppressed.
 *
 * It would be maximally flexible if we could directly derive an SDK version
 * parameter from the "honest" ...MAX_ALLOWED, but cpp has no way to do that
 * and survive the possible redefinition of ...MAX_ALLOWED in step 4, or
 * the possible removal of the relevant version macro in step 5.  Hence,
 * all decisions related to the SDK version need to be made in step 3.  The
 * #if/#elif chain derives an SDK version number, but only with respect to
 * the "major" version (ignoring the least significant digit).  Hence, it's
 * called "MAJOR" rather than "VERSION".
 *
 * NOTE: Some "mismatched SDK" configurations may produce compiler warnings.
 * These are not the fault of this header, and usually aren't fatal unless
 * treated as errors.
 *
 * In the non-Apple case, we avoid AvailabilityMacros.h, and just define our
 * flags for the "minimally hackish" case.
 */

#if __APPLE__

/* First set up the condition for applying the "earlier SDK" hack. */

#ifndef _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED
#define __MPLS_MIN_SDK_ALLOWED 999999
#else
/* Minimum allowable value is 1000 (10.0) */
#if _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED < 1000
#define __MPLS_MIN_SDK_ALLOWED 1000
#else
#define __MPLS_MIN_SDK_ALLOWED _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED
#endif
#endif /* _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED defined */

/* If we already have ...MAX_ALLOWED, we can't do anything to get it. */
#ifndef MAC_OS_X_VERSION_MAX_ALLOWED

/* Otherwise, obtain it, possibly unforced */

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    && __MPLS_MIN_SDK_ALLOWED < __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define MAC_OS_X_VERSION_MIN_REQUIRED __MPLS_MIN_SDK_ALLOWED
#define __MPLS_NEED_MIN_REQUIRED_FIXUP 1
#endif

/* Make sure version macros get defined in 11.x+ SDKs. */
#if !((!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) \
      || defined(_DARWIN_C_SOURCE))
#define __MPLS_SDK11_BLOCKS_VERSION_MACROS
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE 1
#define __MPLS_DARWIN_C_UNDEF
#endif
#endif /* 11.x+ SDK would block version macros */

#include <AvailabilityMacros.h>

/* Now restore _DARWIN_C_SOURCE to the way it was. */
#ifdef __MPLS_DARWIN_C_UNDEF
#undef _DARWIN_C_SOURCE
#undef __MPLS_DARWIN_C_UNDEF
#endif

#endif /* MAC_OS_X_VERSION_MAX_ALLOWED undef */

/* Define the major SDK version via an if/elif chain. */
/* Add new entries as needed. */

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1040
#error Unsupported or incorrectly obtained SDK version
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 1050
#define __MPLS_SDK_MAJOR 1040
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 1060
#define __MPLS_SDK_MAJOR 1050
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 1070
#define __MPLS_SDK_MAJOR 1060
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 1080
#define __MPLS_SDK_MAJOR 1070
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 1090
#define __MPLS_SDK_MAJOR 1080
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101000
#define __MPLS_SDK_MAJOR 1090
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101100
#define __MPLS_SDK_MAJOR 101000
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101200
#define __MPLS_SDK_MAJOR 101100
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101300
#define __MPLS_SDK_MAJOR 101200
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101400
#define __MPLS_SDK_MAJOR 101300
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 101500
#define __MPLS_SDK_MAJOR 101400
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 110000
#define __MPLS_SDK_MAJOR 101500
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 120000
#define __MPLS_SDK_MAJOR 110000
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 130000
#define __MPLS_SDK_MAJOR 120000
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 140000
#define __MPLS_SDK_MAJOR 130000
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 150000
#define __MPLS_SDK_MAJOR 140000
#elif MAC_OS_X_VERSION_MAX_ALLOWED < 160000
#define __MPLS_SDK_MAJOR 150000
#else
#error Unknown SDK version
#endif

/*
 * Workaround for broken 15.0 SDK
 *
 * Apple screwed up and failed to update AvailabilityMacros.h for the
 * 15.0 SDK, causing it to appear to be the 14.x SDK.  However, the 15.0
 * macro was added to AvailabilityVersions.h, so we can check for
 * that as a workaround, when the apparent version is 14.x.  In this
 * case, AvailabilityVersions.h has already been included, so we don't
 * need to do it again.
 */

#if __MPLS_SDK_MAJOR == 140000
  #ifdef MAC_OS_VERSION_15_0
    #undef __MPLS_SDK_MAJOR
    #define __MPLS_SDK_MAJOR 150000
  #endif
#endif /* __MPLS_SDK_MAJOR == 140000 */

/* Then correct our munging, if necessary. */

/* First the ...MIN_REQUIRED hack. */
#ifdef __MPLS_NEED_MIN_REQUIRED_FIXUP

#undef MAC_OS_X_VERSION_MIN_REQUIRED
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define MAC_OS_X_VERSION_MIN_REQUIRED \
        __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define MAC_OS_X_VERSION_MIN_REQUIRED 1040
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_MIN_REQUIRED
#undef MAC_OS_X_VERSION_MAX_ALLOWED
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_MIN_REQUIRED
#endif

#undef __MPLS_NEED_MIN_REQUIRED_FIXUP
#endif /* __MPLS_NEED_MIN_REQUIRED_FIXUP */

/* Then the version macro undisabled definition hack. */
#ifdef __MPLS_SDK11_BLOCKS_VERSION_MACROS

#if __MPLS_SDK_MAJOR >= 110000

/* Version macros conditionally provided by 11.x+ SDKs. */
/* Add new entries as needed, lest unwanted defs linger. */
#undef MAC_OS_VERSION_11_0
#undef MAC_OS_VERSION_11_1
#undef MAC_OS_VERSION_11_3
#undef MAC_OS_VERSION_11_4
#undef MAC_OS_VERSION_11_5
#undef MAC_OS_VERSION_11_6
#undef MAC_OS_VERSION_12_0
#undef MAC_OS_VERSION_12_1
#undef MAC_OS_VERSION_12_2
#undef MAC_OS_VERSION_12_3
#undef MAC_OS_VERSION_12_4
#undef MAC_OS_VERSION_12_5
#undef MAC_OS_VERSION_12_6
#undef MAC_OS_VERSION_12_7
#undef MAC_OS_VERSION_13_0
#undef MAC_OS_VERSION_13_1
#undef MAC_OS_VERSION_13_1
#undef MAC_OS_VERSION_13_2
#undef MAC_OS_VERSION_13_3
#undef MAC_OS_VERSION_13_4
#undef MAC_OS_VERSION_13_5
#undef MAC_OS_VERSION_13_6
#undef MAC_OS_VERSION_14_0
#undef MAC_OS_VERSION_14_1
#undef MAC_OS_VERSION_14_2
#undef MAC_OS_VERSION_14_3
#undef MAC_OS_VERSION_14_4
#undef MAC_OS_VERSION_14_5
#undef MAC_OS_VERSION_15_0
#undef MAC_OS_VERSION_15_1
#undef MAC_OS_VERSION_15_2
#undef MAC_OS_VERSION_15_3
#undef MAC_OS_VERSION_15_4
#undef MAC_OS_VERSION_15_5

#endif /* __MPLS_SDK_MAJOR >= 110000 */

#undef __MPLS_SDK11_BLOCKS_VERSION_MACROS
#endif /* __MPLS_SDK11_BLOCKS_VERSION_MACROS */

#else /* !__APPLE__ */

/* If non-Apple, just assume an "infinitely late" SDK. */

#ifndef __MPLS_SDK_MAJOR
#define __MPLS_SDK_MAJOR 999999
#endif

#endif /* !__APPLE__ */

#endif /* _MACPORTS_SDKVERSION_H_ */
