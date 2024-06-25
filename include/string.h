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

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system string.h */
#include_next <string.h>

/* stpncpy */
#if __MPLS_SDK_SUPPORT_STPNCPY__
__MP__BEGIN_DECLS
extern char *stpncpy(char *dst, const char *src, size_t n);
__MP__END_DECLS
#endif /* __MPLS_SDK_SUPPORT_STPNCPY__ */

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

/*
 * Ordinarily this would just use the usual SDK conditional.  But the
 * __builtin___stpncpy_chk workaround is useful even when the security
 * wrapper is supplied by the SDK, so the condition for supplying our own
 * wrapper is more complicated.
 *
 * Some compilers lack __builtin___stpncpy_chk, requiring a workaround.
 * Handling this is complicated by the fact that support was added to
 * some compilers prior to the __has_builtin() feature that allows directly
 * testing for it.  It's possible to work around this based on the compiler
 * version, but that's further complicated by the fact that Apple added it
 * earlier than the official GCC addition in 4.7.  So, in the absence of
 * the availability of __has_builtin(), we need to see whether either the
 * GCC version or the Apple GCC version is sufficiently recent.
 *
 * Apple didn't get around to adding this logic until the 10.9 SDK, even
 * though stpncpy() was added in 10.7, so the correct (albeit complicated)
 * condition was initially missed.  The condition here is taken directly
 * from secure/_string.h in the 10.9+ SDKs.
 *
 * The workaround here is to define a missing __builtin___stpncpy_chk as
 * a macro pointing at the runtime code, which is less efficient but works.
 *
 * This applies regardless of whether the wrapper comes from here or from
 * a 10.7+ SDK, hence we always define it here.  To make this effective
 * in the 10.7+ SDK case, we use a different name for the inline, which
 * also avoids a duplicate definition issue.
 */

/* Create a proper test for the builtin */
#ifdef __has_builtin
#define __HAVE_BUILTIN_STPNCPY_CHK__ __has_builtin(__builtin___stpncpy_chk)
#else
#define __HAVE_BUILTIN_STPNCPY_CHK__  \
        (__APPLE_CC__ >= 5666     \
         || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#endif

/* Define workaround if needed */
#if !__HAVE_BUILTIN_STPNCPY_CHK__
extern char *__stpncpy_chk(char *dest, const char *src, size_t len,
                           size_t dstlen);
#define __builtin___stpncpy_chk __stpncpy_chk
#undef stpncpy
#endif /* !__HAVE_BUILTIN_STPNCPY_CHK__ */

#if __MPLS_SDK_SUPPORT_STPNCPY__ || !__HAVE_BUILTIN_STPNCPY_CHK__

/* Define the wrapper, possibly replacing the SDK version */
#define stpncpy(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___stpncpy_chk (dest, src, len, __darwin_obsz (dest))	\
   : __mpls_inline_stpncpy_chk (dest, src, len))

static __inline char *
__mpls_inline_stpncpy_chk (char *__restrict __dest,
                           const char *__restrict __src, size_t __len)
{
  return __builtin___stpncpy_chk (__dest, __src, __len, __darwin_obsz(__dest));
}

#endif /* __MPLS_SDK_SUPPORT_STPNCPY__ || !__HAVE_BUILTIN_STPNCPY_CHK__ */

#endif /* _USE_FORTIFY_LEVEL > 0 */

#endif /* _MACPORTS_STRING_H_ */
