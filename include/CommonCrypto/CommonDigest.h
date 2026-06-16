/*
 * Copyright (c) 2026
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

#ifndef _MACPORTS_COMMONCRYPTO_COMMONDIGEST_H_
#define _MACPORTS_COMMONCRYPTO_COMMONDIGEST_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system CommonCrypto/CommonDigest.h */
#include_next <CommonCrypto/CommonDigest.h>

#if __MPLS_SDK_SUPPORT_COMMONDIGEST_ADDS__

/*
 * This adds some digest macros that are missing from the 10.4 header.
 * Note that the 384/512 "discrepancy" is actually correct, due to sharing
 * of the context structure.
 *
 * These definitions are the "functional" style copied from the 10.5 SDK,
 * rather than the "argless" style used in 10.6+.
 */

#ifdef  COMMON_DIGEST_FOR_OPENSSL

#define SHA256_DIGEST_LENGTH    CC_SHA256_DIGEST_LENGTH
#define SHA256_CTX              CC_SHA256_CTX
#define SHA256_Init(c)          CC_SHA256_Init(c)
#define SHA256_Update(c,d,l)    CC_SHA256_Update(c,d,l)
#define SHA256_Final(m, c)      CC_SHA256_Final(m,c)

#define SHA384_DIGEST_LENGTH    CC_SHA384_DIGEST_LENGTH
#define SHA512_CTX              CC_SHA512_CTX
#define SHA384_Init(c)          CC_SHA384_Init(c)
#define SHA384_Update(c,d,l)    CC_SHA384_Update(c,d,l)
#define SHA384_Final(m, c)      CC_SHA384_Final(m,c)

#define SHA512_DIGEST_LENGTH    CC_SHA512_DIGEST_LENGTH
#define SHA512_Init(c)          CC_SHA512_Init(c)
#define SHA512_Update(c,d,l)    CC_SHA512_Update(c,d,l)
#define SHA512_Final(m, c)      CC_SHA512_Final(m,c)

#endif  /* COMMON_DIGEST_FOR_OPENSSL */

#endif /* __MPLS_SDK_SUPPORT_COMMONDIGEST_ADDS__ */

#endif /* _MACPORTS_COMMONCRYPTO_COMMONDIGEST_H_ */
