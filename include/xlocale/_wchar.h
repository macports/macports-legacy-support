/*
 * Copyright (c) 2018 Christian Cornelssen
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

#ifndef _MACPORTS_XLOCALE__WCHAR_H_
#define _MACPORTS_XLOCALE__WCHAR_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system xlocale/_wchar.h */
#include_next <xlocale/_wchar.h>

/* Assume that this was included by wchar.h, which included sys/cdefs.h */

/* Additional functionality provided by:
 * POSIX.1-2008
 */
#if __DARWIN_C_LEVEL >= 200809L

/* wcsncasecmp_l, wcscasecmp_l */
#if __MPLS_SDK_SUPPORT_WCSCASECMP__
__MP__BEGIN_DECLS
extern int wcscasecmp_l(const wchar_t *l, const wchar_t *r, locale_t locale);
extern int wcsncasecmp_l(const wchar_t *l, const wchar_t *r, size_t n, locale_t locale);
__MP__END_DECLS
#endif

#endif /* __DARWIN_C_LEVEL >= 200809L */

#endif /* _MACPORTS_XLOCALE__WCHAR_H_ */
