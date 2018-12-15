
/*
 * Copyright (c) 2018 Ken Cunningham
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

// MP support header
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_WCSNLEN__

#include <wchar.h>

size_t wcsnlen(const wchar_t *s, size_t n)
{
    const wchar_t *z = wmemchr(s, 0, n);
    if (z) n = z-s;
    return n;
}

#endif /* __MP_LEGACY_SUPPORT_WCSNLEN__ */
