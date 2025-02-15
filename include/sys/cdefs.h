/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

#ifndef _MACPORTS_SYS_CDEFS_H_
#define _MACPORTS_SYS_CDEFS_H_

/*
 * Apple renamed _APPLE_C_SOURCE to _DARWIN_C_SOURCE in the 10.5 SDK.
 * Arrange to accept either.
 *
 * Do this before processing cdefs.
 */
#if !defined(_APPLE_C_SOURCE) && defined(_DARWIN_C_SOURCE)
#define _APPLE_C_SOURCE _DARWIN_C_SOURCE
#endif
#if !defined(_DARWIN_C_SOURCE) && defined(_APPLE_C_SOURCE)
#define _DARWIN_C_SOURCE _APPLE_C_SOURCE
#endif

/*
 * Work around bug in some versions of clang (e.g. Xcode 7.2 clang 7).
 * See: https://bugs.llvm.org/show_bug.cgi?id=23435
 *
 * Some versions of clang implement __has_cpp_attribute, but are unable to
 * parse a namespaced argument in C mode.  The official fix is to make
 * __has_cpp_attribute undefined when compiling C.  We can't do that here,
 * but we can override it with a dummy, disabling the resulting warning
 * as we do so.  Since the condition is based on the actual clang bug, no
 * SDK version condition is needed.
 *
 * The first case where this arose was in using the 15.x SDK with the
 * Xcode 7.2 clang 7 compiler on OS 10.10.
 *
 * A similar bug exists in gcc5+, and remained until gcc12, where it was
 * fixed by tolerating the namespaced argument even in C mode, rather than
 * removing __has_cpp_attribute().  The clang workaround is still applicable
 * in principle, but the option to disable the redefinition warning doesn't
 * work in GCC, apparently because it only applies to redefinitions of
 * certain specific macros (not including __has_cpp_attribute).
 *
 * Because the warning is unavoidable in GCC, we only apply the workaround
 * in cases where it's actually necessary, which means GCC < 12 and
 * SDK >= 15.  This is the only case where we care about the SDK version
 * in this header, so the include of sdkversion.h is limited to this case.
 *
 * Note that the case where __has_cpp_attribute is undefined is already
 * handled by the Apple include, so only the inappropriately defined case
 * needs to be handled here.
 */

#if defined(__has_cpp_attribute) && !defined(__cplusplus)
  #if defined (__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wbuiltin-macro-redefined"
    #define __has_cpp_attribute(x) 0
    #pragma clang diagnostic pop
  #elif defined(__GNUC__) && __GNUC__ < 12
    /* Get the SDK version to see if we need the workaround. */
    #include <_macports_extras/sdkversion.h>
    #if __MPLS_SDK_MAJOR >= 150000
      #pragma GCC diagnostic push
      /* The following warning disable doesn't actually work. */
      #pragma GCC diagnostic ignored "-Wbuiltin-macro-redefined"
      #define __has_cpp_attribute(x) 0 /* Probably unavoidable warning here */
      #pragma GCC diagnostic pop
    #endif
  #endif
#endif

/*
 * Work around issues with depending on __has_include().
 *
 * The system sys/cdefs.h may provide an always-false fallback for the
 * __has_include() operator.  But this may cause some files to be
 * inappropriately not included.  Here we set up a flag for this case,
 * which can be used by other headers as needed to fix the fallback.
 *
 * The state is represented by the __MPLS_HAS_INCLUDE_STATUS macro:
 *   -1  undef->undef  Remained undef
 *    0  undef->def    Defined false by sys/cdefs.h
 *    1  def->def      Provided by compiler
 */

/* First capture the initial defined state. */
#ifndef __has_include
#define __MPLS_HAS_INCLUDE_STATUS -1
#else
#define __MPLS_HAS_INCLUDE_STATUS 1
#endif

/*
 * This provides definitions for the __DARWIN_C_* macros for earlier SDKs
 * that don't provide them.  Since the definitions are based on #ifndef,
 * there's no need for explicit SDK version thresholds.
 *
 * Note that all SDKs provide adjustments for _POSIX_C_SOURCE where needed.
 */

/* Include the primary system sys/cdefs.h */
#include_next <sys/cdefs.h>

/* Now update the __has_include() status if needed. */
#if __MPLS_HAS_INCLUDE_STATUS < 0 && defined(__has_include)
#undef __MPLS_HAS_INCLUDE_STATUS
#define __MPLS_HAS_INCLUDE_STATUS 0
#endif

/* The following is copied from the 10.7 SDK, but with additional #ifndefs */

/*
 * Set a single macro which will always be defined and can be used to determine
 * the appropriate namespace.  For POSIX, these values will correspond to
 * _POSIX_C_SOURCE value.  Currently there are two additional levels corresponding
 * to ANSI (_ANSI_SOURCE) and Darwin extensions (_DARWIN_C_SOURCE)
 */
#ifndef __DARWIN_C_ANSI
#define __DARWIN_C_ANSI         010000L
#endif

#ifndef __DARWIN_C_FULL
#define __DARWIN_C_FULL         900000L
#endif

#ifndef __DARWIN_C_LEVEL

#if   defined(_ANSI_SOURCE)
#define __DARWIN_C_LEVEL        __DARWIN_C_ANSI
#elif defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE) && !defined(_NONSTD_SOURCE)
#define __DARWIN_C_LEVEL        _POSIX_C_SOURCE
#else
#define __DARWIN_C_LEVEL        __DARWIN_C_FULL
#endif

#endif /* __DARWIN_C_LEVEL undef */

/* Provide any needed and missing symbol versioning (10.4) */
#ifndef __DARWIN_SUF_64_BIT_INO_T
#if defined(__DARWIN_64_BIT_INO_T) && __DARWIN_64_BIT_INO_T
#define __DARWIN_SUF_64_BIT_INO_T "$INODE64"
#else
#define __DARWIN_SUF_64_BIT_INO_T
#endif
#endif /* __DARWIN_SUF_64_BIT_INO_T undef */
#ifndef __DARWIN_INODE64
#define __DARWIN_INODE64(sym) __asm("_" __STRING(sym) __DARWIN_SUF_64_BIT_INO_T)
#endif

/*
 * Without help from legacy-support, 64-bit-inode functions are unavailable
 * in 10.4, and the 10.4 SDK has no means to generate references to them.
 * However, builds with a 10.5+ SDK and _DARWIN_USE_64_BIT_INODE set will
 * set __DARWIN_64_BIT_INO_T, even on 10.4.  But 32-bit 10.4 builds default
 * to __DARWIN_UNIX03=0, which clears __DARWIN_SUF_64_BIT_INO_T without
 * disabling __DARWIN_64_BIT_INO_T, leading to a mismatch in expected
 * struct stat formats.
 *
 * Originally, we fixed this by disabling __DARWIN_64_BIT_INO_T in this
 * particular case, since there was no support for 64-bit inodes on 10.4,
 * anyway.  But now that limited such support exists, we leave that flag
 * alone and reenable __DARWIN_SUF_64_BIT_INO_T instead.  This allows use
 * of the available 64-bit-inode functions without being forced to
 * enable __DARWIN_UNIX03, which causes many functions to expect nonexistent
 * $UNIX2003 variants.
 *
 * This does not affect 64-bit builds, 10.5+ builds, or builds that
 * set any of the flags enabling Unix compliance.
 */
#if !__DARWIN_UNIX03 && defined(__DARWIN_64_BIT_INO_T) && __DARWIN_64_BIT_INO_T
#undef __DARWIN_SUF_64_BIT_INO_T
#define __DARWIN_SUF_64_BIT_INO_T "$INODE64"
#endif

#endif /* _MACPORTS_SYS_CDEFS_H_ */
