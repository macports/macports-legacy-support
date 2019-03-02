
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

#ifndef _MACPORTS_STDIO_H_
#define _MACPORTS_STDIO_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system time.h */
#include_next <stdio.h>

/* getline */
#if __MP_LEGACY_SUPPORT_GETLINE__

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
  extern ssize_t getdelim(char **lineptr, size_t *n, int delimiter, FILE *fp);
  extern ssize_t getline (char **lineptr, size_t *n, FILE *stream);
#ifdef __cplusplus
}
#endif
#endif /*  __MP_LEGACY_SUPPORT_GETLINE__ */

#endif /* _MACPORTS_STDIO_H_ */
