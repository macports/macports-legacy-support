
/*
 * Copyright (c) 2010 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2018 Ken Cunningham <kencu@macports.org>
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


/* realpath wrap */
#if __MP_LEGACY_SUPPORT_REALPATH_WRAP__
/* we need a way to isolate ourselves from our header wrapping while building */
#ifndef __BUILDING_MP_LEGACY_SUPPORT_REALPATH_WRAP__

/* we are going to move the old realpath definition out of the way */
#undef realpath
#define realpath realpath_macports_original

#endif /*__BUILDING_MP_LEGACY_SUPPORT_REALPATH_WRAP__ */
#endif /*__MP_LEGACY_SUPPORT_REALPATH_WRAP__*/


/* Include the primary system stdlib.h */
#include_next <stdlib.h>


/* realpath wrap */
#if __MP_LEGACY_SUPPORT_REALPATH_WRAP__
#ifndef __BUILDING_MP_LEGACY_SUPPORT_REALPATH_WRAP__

/* and now define realpath as our new wrapped function */
#undef realpath
#define realpath macports_legacy_realpath

#ifdef __cplusplus
extern "C" {
#endif
  extern char * macports_legacy_realpath(const char *, void *);
#ifdef __cplusplus
}
#endif
#endif /*__BUILDING_MP_LEGACY_SUPPORT_REALPATH_WRAP__ */
#endif /*__MP_LEGACY_SUPPORT_REALPATH_WRAP__*/


/* posix_memalign */
#if __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__

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

#ifdef __cplusplus
extern "C" {
#endif
  extern int posix_memalign(void **memptr, size_t alignment, size_t size);
#ifdef __cplusplus
}
#endif

#endif /*  __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__ */

#endif /* _MACPORTS_STDLIB_H_ */
