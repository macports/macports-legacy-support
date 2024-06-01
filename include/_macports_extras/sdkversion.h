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
 * There's an additional complication if ...MAX_ALLOWED is already defined
 * initially.  That means that either it was explicitly defined in some
 * fashion, or AvailabilityMacros.h was already included before.  In this
 * case, the above workaround doesn't work, so the best we can do is to take
 * ...MAX_ALLOWED at face value and hope that it works.
 *
 * THUS: If supporting the "older SDK" case is important, this header should
 * be included before anything that might include AvailabilityMacros.h.
 *
 * In the non-Apple case, we avoid AvailabilityMacros.h, and just define our
 * flags for the "minimally hackish" case.
 *
 * It would be maximally flexible if we could simply derive an SDK version
 * parameter from the "honest" ...MAX_ALLOWED, but cpp has no way to do that
 * and survive the possible redefinition of ...MAX_ALLOWED in step 4.  Hence,
 * all decisions related to the SDK version need to be made in step 3.
 *
 * NOTE: Some "mismatched SDK" configurations may produce compiler warnings.
 * These are not the fault of this header, and usually aren't fatal unless
 * treated as errors.
 */

#if __APPLE__

/* First obtain MAC_OS_X_VERSION_MAX_ALLOWED, possibly unforced */

#undef __MPLS_NEED_MIN_REQUIRED_FIXUP

#ifndef MAC_OS_X_VERSION_MAX_ALLOWED

#define __MPLS_NEED_MIN_REQUIRED_FIXUP 1

/* Lowest allowable value is for 10.0 */
#define MAC_OS_X_VERSION_MIN_REQUIRED 1000

#include <AvailabilityMacros.h>

#endif /* MAC_OS_X_VERSION_MAX_ALLOWED */

/* Set up our flags here for any needed version thresholds */

/* Then correct our munging, if necessary */

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

#endif /* __MPLS_NEED_MIN_REQUIRED_FIXUP */

#else /* !__APPLE__ */

/* If non-Apple, just assume an "infinitely late" SDK */

#endif /* !__APPLE__ */

#endif /* _MACPORTS_SDKVERSION_H_ */
