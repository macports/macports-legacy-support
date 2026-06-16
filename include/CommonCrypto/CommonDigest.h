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

#if __MPLS_SDK_SUPPORT_COMMONDIGEST_ARGLESS__ \
    && (!defined(_MACPORTS_LEGACY_DISABLE_ARGLESS_DIGEST) \
        || !_MACPORTS_LEGACY_DISABLE_ARGLESS_DIGEST)
/*
 * Beginning in 10.6, the digest macros switched to the "argless" form.
 * We provide redefinitions to match, but with an option to disable that
 * for programs that expect the old <10.6 behavior.
 */

#ifdef  COMMON_DIGEST_FOR_OPENSSL

#undef MD2_Init
#define MD2_Init            CC_MD2_Init
#undef MD2_Update
#define MD2_Update          CC_MD2_Update
#undef MD2_Final
#define MD2_Final           CC_MD2_Final

#undef MD4_Init
#define MD4_Init            CC_MD4_Init
#undef MD4_Update
#define MD4_Update          CC_MD4_Update
#undef MD4_Final
#define MD4_Final           CC_MD4_Final

#undef MD5_Init
#define MD5_Init            CC_MD5_Init
#undef MD5_Update
#define MD5_Update          CC_MD5_Update
#undef MD5_Final
#define MD5_Final           CC_MD5_Final

#undef SHA1_Init
#define SHA1_Init           CC_SHA1_Init
#undef SHA1_Update
#define SHA1_Update         CC_SHA1_Update
#undef SHA1_Final
#define SHA1_Final          CC_SHA1_Final

#undef SHA224_Init
#define SHA224_Init         CC_SHA224_Init
#undef SHA224_Update
#define SHA224_Update       CC_SHA224_Update
#undef SHA224_Final
#define SHA224_Final        CC_SHA224_Final

#undef SHA256_Init
#define SHA256_Init         CC_SHA256_Init
#undef SHA256_Update
#define SHA256_Update       CC_SHA256_Update
#undef SHA256_Final
#define SHA256_Final        CC_SHA256_Final

#undef SHA384_Init
#define SHA384_Init         CC_SHA384_Init
#undef SHA384_Update
#define SHA384_Update       CC_SHA384_Update
#undef SHA384_Final
#define SHA384_Final        CC_SHA384_Final

#undef SHA512_Init
#define SHA512_Init         CC_SHA512_Init
#undef SHA512_Update
#define SHA512_Update       CC_SHA512_Update
#undef SHA512_Final
#define SHA512_Final        CC_SHA512_Final

#endif  /* COMMON_DIGEST_FOR_OPENSSL */

#ifdef	COMMON_DIGEST_FOR_RFC_1321

#undef MD5_Init
#define MD5Init             CC_MD5_Init
#undef MD5Update
#define MD5Update           CC_MD5_Update
#undef MD5Final
void MD5Final (unsigned char [16], MD5_CTX *);

#endif	/* COMMON_DIGEST_FOR_RFC_1321 */

#endif /* __MPLS_SDK_SUPPORT_COMMONDIGEST_ARGLESS__ && !disabled */

#endif /* _MACPORTS_COMMONCRYPTO_COMMONDIGEST_H_ */
