
/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

#ifndef _MACPORTS_STRING_H_
#define _MACPORTS_STRING_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system string.h */
#include_next <string.h>

/* stpncpy */
/*
 * If we're building with a 10.7+ SDK, stpncpy may have already been defined as
 * a macro. In that case, leave it as is.  This not only leaves it as provided
 * by the SDK, but also informs the decision below.
 */
#if __MP_LEGACY_SUPPORT_STPNCPY__ && !defined(stpncpy)
__MP__BEGIN_DECLS
extern char *stpncpy(char *dst, const char *src, size_t n);
__MP__END_DECLS
#endif /* __MP_LEGACY_SUPPORT_STPNCPY__ && !defined(stpncpy) */

/* strnlen */
#if __MP_LEGACY_SUPPORT_STRNLEN__
__MP__BEGIN_DECLS
extern size_t strnlen(const char *s, size_t maxlen);
__MP__END_DECLS
#endif

/* strndup */
#if __MP_LEGACY_SUPPORT_STRNDUP__
__MP__BEGIN_DECLS
extern char *strndup(const char *s, size_t n);
__MP__END_DECLS
#endif

/* memmem */
#if __MP_LEGACY_SUPPORT_MEMMEM__
__MP__BEGIN_DECLS
extern void *
memmem(const void *l, size_t l_len, const void *s, size_t s_len);
__MP__END_DECLS
#endif

/*
 * Security wrapper support
 *
 * Rather than pushing this off into an added secure/_string.h, we just do it
 * here directly.
 * If security wrappers are wanted, the SDK string.h has already included
 * secure/_common.h, and _USE_FORTIFY_LEVEL has been appropriately defined.
 * Otherwise, _USE_FORTIFY_LEVEL is undefined.
 */
#if defined(_USE_FORTIFY_LEVEL) && _USE_FORTIFY_LEVEL > 0

/* stpncpy */
/* Note the defense against building with a 10.7+ SDK, as above. */
#if __MP_LEGACY_SUPPORT_STPNCPY__ && !defined(stpncpy)

/*
 * GCC 4.2 for 10.5 lacks __builtin___stpncpy_chk, even though GCC 4.2
 * for 10.6 has it.  In the absence of a reasonable way to check for compiler
 * support directly, we rely on the OS version for the decision.  Note that
 * the security wrapper mechanism isn't enabled by default on 10.5, anyway,
 * but this allows it to work (inefficiently) if it's enabled explicitly.
 */
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060
extern char *__stpncpy_chk(char *dest, const char *src, size_t len,
                           size_t dstlen);
#define __builtin___stpncpy_chk __stpncpy_chk
#endif /* OS <10.6 */

#define stpncpy(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___stpncpy_chk (dest, src, len, __darwin_obsz (dest))	\
   : __inline_stpncpy_chk (dest, src, len))

static __inline char *
__inline_stpncpy_chk (char *__restrict __dest, const char *__restrict __src,
		      size_t __len)
{
  return __builtin___stpncpy_chk (__dest, __src, __len, __darwin_obsz(__dest));
}

#endif /* __MP_LEGACY_SUPPORT_STPNCPY__ && !defined(stpncpy) */

#endif /* _USE_FORTIFY_LEVEL > 0 */

#endif /* _MACPORTS_STRING_H_ */
