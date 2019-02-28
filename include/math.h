
/*
 * Copyright (c) 2010 Chris Jones <jonesc@macports.org>
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

#if __MP_LEGACY_SUPPORT_LLROUND__

/* For definition of __*_DECLS */
#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * These functions are present in the system math library but their
 * prototypes might not be declared under some circumstances. Declare
 * them here anyway.
 */

/*
 * this is the same condition that defines the function prototypes in
 * the system <math.h>.
 */
#if ( defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || ! defined( __STRICT_ANSI__ )  || ! defined( __GNUC__ )
#else
extern long long int llrint   ( double );
extern long long int llrintf  ( float );
extern long long int llrintl  ( long double );

extern long long int llround  ( double );
extern long long int llroundf ( float );
extern long long int llroundl ( long double );
#endif

__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_LLROUND__ */

/*
 * Include the next math.h, which might be from the primary system or
 * it might be within GCC's c or c++ (yup!) headers
 */

/*
 * If the GCC <math.h> header exists, then tell it: (1) to include the
 * next <math.h>, which should be from the system; and (2) to not use
 * it's <math.h> yet, because it basically wraps <cmath> and we need
 * to keep everything herein focused on just <math.h>. If the user
 * wants <cmath>, they should #include that specific header.
 */

/* store prior values for _GLIBCXX_MATH_H and _GLIBCXX_INCLUDE_NEXT_C_HEADERS */

#ifdef L_GLIBCXX_MATH_H
#undef L_GLIBCXX_MATH_H
#endif
#ifdef _GLIBCXX_MATH_H
#define L_GLIBCXX_MATH_H _GLIBCXX_MATH_H
#endif
#define _GLIBCXX_MATH_H 1

#ifdef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif
#ifdef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#define L_GLIBCXX_INCLUDE_NEXT_C_HEADERS _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif
#define _GLIBCXX_INCLUDE_NEXT_C_HEADERS 1

#include_next <math.h>

/* restore prior values for _GLIBCXX_MATH_H and _GLIBCXX_INCLUDE_NEXT_C_HEADERS */

#undef _GLIBCXX_MATH_H
#ifdef L_GLIBCXX_MATH_H
#define _GLIBCXX_MATH_H L_GLIBCXX_MATH_H
#undef L_GLIBCXX_MATH_H
#endif

#undef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#ifdef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#define _GLIBCXX_INCLUDE_NEXT_C_HEADERS L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif

#endif /* _MACPORTS_MATH_H_ */
