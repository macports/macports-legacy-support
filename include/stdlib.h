
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

/* Include the primary system stdlib.h */
#include_next <stdlib.h>

/* MP support header */
#include "MacportsLegacySupport.h"

/* posix_memalign */
#if __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__

typedef long ssize_t;	
#ifdef __cplusplus
extern "C" {
#endif
  extern int posix_memalign(void **memptr, size_t alignment, size_t size);
#ifdef __cplusplus
}
#endif

#endif /*  __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__ */

#endif /* _MACPORTS_STDLIB_H_ */
