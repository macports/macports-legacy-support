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

/*
 * This is a test of the CommonDigest header.  It verifies that the definitions
 * missing from 10.4 are present, just with dummy uses.  This includes calling
 * the functions, but not checking the results.
 *
 * As a test of the "argless" versions of the macros used in 10.6+ and
 * backported here, we also verify that we can take pointers to all the
 * functions.
 *
 * Note that SHA224 is only supported on 10.5+.  Since we don't currently
 * fix that, we filter the tests accordingly.
 */

/* Get target OS version */
#include <_macports_extras/targetos.h>

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#include <libgen.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>

static const char *test_data = "The Quick Brown Fox";

#define DIGESTS0 \
  DIGEST(MD2,MD2,MD2) \
  DIGEST(MD4,MD4,MD4) \
  DIGEST(MD5,MD5,MD5) \
  DIGEST(SHA1,SHA,SHA) \
  DIGEST(SHA256,SHA256,SHA256) \
  DIGEST(SHA384,SHA512,SHA384) \
  DIGEST(SHA512,SHA512,SHA512) \

#if __MPLS_SDK_MAJOR >= 1050 && __MPLS_TARGET_OSVER >= 1050
  #define DIGESTS \
    DIGESTS0 \
    DIGEST(SHA224,SHA256,SHA224)
#else
  #define DIGESTS \
    DIGESTS0
#endif

#define DIGEST(type,ctype,ltype) \
  static __typeof__(type ## _Init) *type ## _Init_p; \
  static __typeof__(type ## _Update) *type ## _Update_p; \
  static __typeof__(type ## _Final) *type ## _Final_p; \

DIGESTS
#undef DIGEST

#define DIGEST(type,ctype,ltype) \
  static void \
  test_ ## type(void) \
  { \
    ctype ## _CTX ctx; \
    uint8_t digest[ltype ## _DIGEST_LENGTH]; \
 \
    type ## _Init(&ctx); \
    type ## _Update(&ctx, (uint8_t *) test_data, strlen(test_data)); \
    type ## _Final(digest, &ctx); \
  } \

DIGESTS
#undef DIGEST

int
main(int argc, char *argv[])
{
  (void) argc;

  #define DIGEST(type,ctype,ltype) \
    (void) type ## _Init_p; \
    (void) type ## _Update_p; \
    (void) type ## _Final_p;
  DIGESTS
  #undef DIGEST

  #define DIGEST(type,ctype,ltype) test_ ## type();
  DIGESTS
  #undef DIGEST

  printf("%s succeeded\n", basename(argv[0]));
  return 0;
}
