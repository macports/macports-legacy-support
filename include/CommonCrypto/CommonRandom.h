/*
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

#ifndef _MACPORTS_COMMONCRYPTO_COMMONRANDOM_H_
#define _MACPORTS_COMMONCRYPTO_COMMONRANDOM_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_SUPPORT_CCRANDOM__

typedef CCCryptorStatus CCRNGStatus;

#else /* !__MPLS_SDK_SUPPORT_CCRANDOM__ */

/* Include the primary system CommonCrypto/CommonRandom.h */
#include_next <CommonCrypto/CommonRandom.h>

#endif /* !__MPLS_SDK_SUPPORT_CCRANDOM__ */

/*
 * We need to provide our prototype whenever we don't have the standard
 * function, in order to remap the name in the "mismatched SDK" case.
 * Hence the "LIB" instead of "SDK.
 *
 * This relies on being able to override the prototype with a new one,
 * which is eqivalent execpt for the "__asm()__" part.
 */

#if __MPLS_LIB_SUPPORT_CCRANDOM__ \
    && defined(_MACPORTS_LEGACY_ALLOW_INSECURE_CCRANDOM) \
    && _MACPORTS_LEGACY_ALLOW_INSECURE_CCRANDOM

__MP__BEGIN_DECLS

CCRNGStatus CCRandomGenerateBytes(void *bytes, size_t count)
            __asm("___mpls_insecure_CCRandomGenerateBytes");

__MP__END_DECLS

#endif /* __MPLS_LIB_SUPPORT_CCRANDOM__ ... */

#endif /* _MACPORTS_COMMONCRYPTO_COMMONRANDOM_H_ */
