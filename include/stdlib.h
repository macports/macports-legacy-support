/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

#ifndef _MACPORTS_STDLIB_H_
#define _MACPORTS_STDLIB_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Work around recent compilers that treat undefineds as errors. */
#if __MPLS_SDK_MAJOR >= 101200
  #ifndef TARGET_OS_EMBEDDED
  #define TARGET_OS_EMBEDDED 0
  #endif
  #ifndef TARGET_OS_IPHONE
  #define TARGET_OS_IPHONE 0
  #endif
#endif

/* For certain added calls, fake out the availability stuff */
/* This is only applicable with a mismatched SDK (LIB & !SDK) */

#if __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ && !__MPLS_SDK_SUPPORT_ALIGNED_ALLOC__
#define aligned_alloc __mpls_dummy_aligned_alloc__
#endif /* __MPLS_LIB_SUPPORT_OPEN_MEMSTREAM__ ... */

/* Include the primary system stdlib.h */
#include_next <stdlib.h>

/* Undo kludge macros */

#if __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ && !__MPLS_SDK_SUPPORT_ALIGNED_ALLOC__
#undef aligned_alloc
#endif /* __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ ... */

/* posix_memalign */
#if __MPLS_SDK_SUPPORT_POSIX_MEMALIGN__

/*
 * [XSI] The ssize_t and size_t types shall be defined as described
 * in <sys/types.h>.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif

#ifndef	_SSIZE_T
#define	_SSIZE_T
typedef	__darwin_ssize_t	ssize_t;
#endif

__MP__BEGIN_DECLS
extern int posix_memalign(void **memptr, size_t alignment, size_t size);
__MP__END_DECLS

#endif /*  __MPLS_SDK_SUPPORT_POSIX_MEMALIGN__ */

/* aligned_alloc */

/* Apple makes aligned_alloc conditional on the language version */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) \
    || (defined(__cplusplus) && __cplusplus >= 201703L)
#define __MPLS_LANGUAGE_HAS_ALIGNED_ALLOC__ 1
#else
#define __MPLS_LANGUAGE_HAS_ALIGNED_ALLOC__ 0
#endif

/* Allow an override */
#if defined(_MACPORTS_LEGACY_ALLOW_ALIGNED_ALLOC) \
    && _MACPORTS_LEGACY_ALLOW_ALIGNED_ALLOC
#define __MPLS_FORCE_ALIGNED_ALLOC 1
#else
#define __MPLS_FORCE_ALIGNED_ALLOC 0
#endif

/* Supply the prototype if it's desired and missing, but not otherwise */
/* Note LIB rather than SDK for consistency with macro kludge above */
#if (__MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ \
     && (__MPLS_LANGUAGE_HAS_ALIGNED_ALLOC__ || __MPLS_FORCE_ALIGNED_ALLOC)) \
    || (!__MPLS_LANGUAGE_HAS_ALIGNED_ALLOC__ && __MPLS_FORCE_ALIGNED_ALLOC)

#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) \
    || defined(_DARWIN_C_SOURCE))
__MP__BEGIN_DECLS
void *aligned_alloc(size_t __alignment, size_t __size);
__MP__END_DECLS
#endif

#endif  /* __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ ... */

#undef __MPLS_LANGUAGE_HAS_ALIGNED_ALLOC__
#undef __MPLS_FORCE_ALIGNED_ALLOC

/* arc4random */
#if __MPLS_SDK_SUPPORT_ARC4RANDOM__

#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif
#ifndef _UINT32_T
#define _UINT32_T
typedef unsigned int         uint32_t;
#endif

/*
 * Generate and return a uniformly random 32-bit quantity with an
 * upper bound of 'upper_bound'
 */
__MP__BEGIN_DECLS
extern uint32_t arc4random_uniform( uint32_t upper_bound );
__MP__END_DECLS

/*
 * Generate 'n' random bytes and put them in 'buf'.
 */
__MP__BEGIN_DECLS
extern void arc4random_buf( void* buf, size_t n );
__MP__END_DECLS

#endif /*  __MPLS_SDK_SUPPORT_ARC4RANDOM__ */

#endif /* _MACPORTS_STDLIB_H_ */
