/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2019 Michael Dickens <michaelld@macports.org>
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

#ifndef _MACPORTS_MATH_H_
#define _MACPORTS_MATH_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_SUPPORT_LLROUND__

__MP__BEGIN_DECLS

/*
 * These functions are present in the system math library but their
 * prototypes might not be declared under some circumstances. Declare
 * them here anyway.
 */

/*
 * this is the same condition that defines the function prototypes in
 * the GCC <math.h>.
 */
#if !(__DARWIN_NO_LONG_LONG)
extern long long int llrint   ( double );
extern long long int llrintf  ( float );
extern long long int llrintl  ( long double );

extern long long int llround  ( double );
extern long long int llroundf ( float );
extern long long int llroundl ( long double );
#endif

__MP__END_DECLS

/*
 * If the GCC <math.h> header exists, then tell it: (1) to include the
 * next <math.h>, which should be from the system; and (2) to not use
 * it's <math.h> yet, because it basically wraps <cmath> and we need
 * to keep everything herein focused on just <math.h>. If the user
 * wants <cmath>, they should #include that specific header.
 */

#undef L_GLIBCXX_MATH_H
#ifndef _GLIBCXX_MATH_H
#define L_GLIBCXX_MATH_H 1
#define _GLIBCXX_MATH_H 1
#endif

#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#ifndef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#define L_GLIBCXX_INCLUDE_NEXT_C_HEADERS 1
#define _GLIBCXX_INCLUDE_NEXT_C_HEADERS 1
#endif

#endif /* __MPLS_SDK_SUPPORT_LLROUND__ */

/*
 * Fix _Float16 issue with 15.x SDK.
 *
 * The 15.0 SDK introduced a set of half-precision floating-point functions
 * (first available in macOS 15.0) without including any support for older
 * compilers that lack the _Float16 type.  With such compilers, the prototypes
 * for those functions are syntactically illegal, resulting in fatal errors
 * in any build that uses math.h, regardless of whether it uses any of these
 * functions.  Here we determine whether it's a 15.x+ SDK and such a compiler,
 * so we can provide a dummy definition for _Float16 to make the declarations
 * work.
 *
 * There are actually two separate issues with _Float16.  The first is whether
 * it's recognized as a type at all.  The second is whether it's supported.
 * Unlike all other types in C, the latter is architecture-specific.  When
 * it's not supported on the target architecture, any attempt to use it, while
 * syntactically legal, still results in a fatal error.  Complicating things
 * further is that many versions of clang and many versions of gcc improperly
 * consider _Float16 to be unsupported on x86.
 *
 * With clang, the mere recognition of the type can be tested in a backhanded
 * way without resorting to version conditionals through the use of the
 * __is_identifier() intrinsic.  Since the purpose of this operator is to
 * determine whether its operand is a valid *user* identifier, it's false for
 * all reserved words, including recognized types.  Versions of clang that
 * are too old to have __is_identifier() are also too old to recognize the
 * _Float16 type, so it can reliably be used to test whether it's recognized
 * as a type.  This, however, doesn't answer the support question.
 *
 * All versions of clang that recognize _Float16 as a type support it for
 * arm64, but Apple clang doesn't support it on x86 until clang 10, and
 * MacPorts clang doesn't support it on x86 until clang 15.  It's never
 * supported on ppc or ppc64.
 *
 * With gcc, there's no __is_identifier feature, so our only option is a
 * hard-coded version threshold.  Gcc versions 7 and later recognize the type,
 * but don't actually support it on x86_64 until version 12.  There's no
 * Apple gcc for arm64, but MacPorts gcc >=10 is available and supports
 * _Float16 on arm64.  MacPorts gcc <10 is currently unavailable for arm64,
 * but we assume that the architecture issue is similarly irrelevant for
 * arm64, and hence that _Float16 is potentially legal for arm64 on gcc >=7.
 *
 * The logic below handles the recognition and support issues separately,
 * for better clarity.  It also defines __MPLS_FLOAT16_STATUS as follows:
 *   -1    Not recognized
 *    0    Recognized, but not supported
 *    1    Recognized and supported
 * This is set up regardless of SDK version, but only acted upon for SDK 15.x+.
 *
 * In failing cases, we define _Float16 as a macro rather than a type, for
 * maximum flexibility.  This is suppressed if it's already defined as a macro
 * when we get here.
 *
 * When this hack is needed, we need to provide *some* compiler-supported type
 * as a substitute.  The reasonable candidates are 'short', which has the right
 * memnory size but the wrong "character", or 'float', which has the wrong
 * memory size but the right "character".  Since actually using any of these
 * functions won't really work properly in either case, we opt for 'short'.
 *
 * If this hack is needed and the target OS is >=15.x, then we give a warning
 * about the compiler problem, though we have no way of knowing whether the
 * code actually uses any of these functions.  If the target OS is <15.x, then
 * we assume that the code doesn't use these functions, since they're not
 * available prior to 15.x, and don't bother with the warning.
 *
 * Note that all this has to happen *before* the include_next, so that _Float16
 * is usable while the system math.h is being processed.
 */

#undef __MPLS_FLOAT16_STATUS

#ifdef __is_identifier
  #if !__is_identifier(_Float16)
    #define __MPLS_FLOAT16_STATUS 0
  #else
    #define __MPLS_FLOAT16_STATUS -1
  #endif
#elif !defined(__clang_major__) && defined(__GNUC__)
  #if __GNUC__ >= 7
    #define __MPLS_FLOAT16_STATUS 0
  #else
    #define __MPLS_FLOAT16_STATUS -1
  #endif
#else
  #define __MPLS_FLOAT16_STATUS -1
#endif

#if __MPLS_FLOAT16_STATUS == 0
  #if defined(__arm64__)
    #undef __MPLS_FLOAT16_STATUS
    #define __MPLS_FLOAT16_STATUS 1
  #elif defined(__x86_64__) || defined(__i386__)
    #if defined(__clang_major__)
      #if __clang_major__ >= 15
        #undef __MPLS_FLOAT16_STATUS
        #define __MPLS_FLOAT16_STATUS 1
      #elif __clang_major__ >= 10 && defined(__apple_build_version__)
        #undef __MPLS_FLOAT16_STATUS
        #define __MPLS_FLOAT16_STATUS 1
      #endif
    #elif defined(__GNUC__)
      #if __GNUC__ >= 12
        #undef __MPLS_FLOAT16_STATUS
        #define __MPLS_FLOAT16_STATUS 1
      #endif
    #endif
  #endif /* Unsupported arch */
#endif

#if __MPLS_SDK_MAJOR >= 150000 && !defined(_Float16)

#if __MPLS_FLOAT16_STATUS <= 0
  #if __MPLS_TARGET_OSVER >= 150000
    #warning Compiler doesn't support _Float16 - relevant functions won't work
  #endif
  #define _Float16 short
#endif

#endif /* __MPLS_SDK_MAJOR >= 150000 && !defined(_Float16) */

/*
 * Include the next math.h, which might be from the primary system or
 * it might be within GCC's c or c++ (yup!) headers
 */

#include_next <math.h>

#if __MPLS_SDK_SUPPORT_COSSIN__

/* Following is borrowed from math.h on macOS 10.9+ */

/*  __sincos and __sincosf were introduced in OSX 10.9 and iOS 7.0.  When
    targeting an older system, we simply split them up into discrete calls
    to sin( ) and cos( ).  */

__MP__BEGIN_DECLS
extern void __sincosf(float __x, float *__sinp, float *__cosp);
extern void __sincos(double __x, double *__sinp, double *__cosp);
__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_COSSIN__ */

#if __MPLS_SDK_SUPPORT_LLROUND__

#ifdef L_GLIBCXX_MATH_H
#undef L_GLIBCXX_MATH_H
#undef _GLIBCXX_MATH_H
#endif

#ifdef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif

#endif /* __MPLS_SDK_SUPPORT_LLROUND__ */

#endif /* _MACPORTS_MATH_H_ */
