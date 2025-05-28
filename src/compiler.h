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

#ifndef __MACPORTS_COMPILER_H
#define __MACPORTS_COMPILER_H

/* Macros for compiler-specific features */

/* Branch prediction hints */
#ifdef __has_builtin
  #if __has_builtin(__builtin_expect)
    #define MPLS_EXPECT(x, v) __builtin_expect((x), (v))
    #define MPLS_FASTPATH(x) ((__typeof__(x))MPLS_EXPECT((long)(x), ~0l))
    #define MPLS_SLOWPATH(x) ((__typeof__(x))MPLS_EXPECT((long)(x), 0l))
  #endif
#endif  /* __has_builtin */

#ifndef MPLS_EXPECT
  #define MPLS_EXPECT(x, v) (x)
  #define MPLS_FASTPATH(x) (x)
  #define MPLS_SLOWPATH(x) (x)
#endif

#endif /* __MACPORTS_COMPILER_H */
