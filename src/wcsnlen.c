/*
 * Adapted for MacportsLegacySupport from:
 *  https://git.musl-libc.org/cgit/musl/tree/src/string/wcsnlen.c
 * License text (excerpt below):
 *  https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 *
 * musl as a whole is licensed under the following standard MIT license:
 *
 * Copyright © 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// MP support header
#include "MacportsLegacySupport.h"
#if __MPLS_LIB_SUPPORT_WCSNLEN__

#include <wchar.h>

size_t wcsnlen(const wchar_t *s, size_t n)
{
    const wchar_t *z = wmemchr(s, 0, n);
    if (z) n = z-s;
    return n;
}

#endif /* __MPLS_LIB_SUPPORT_WCSNLEN__ */
