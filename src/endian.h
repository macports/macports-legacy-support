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

#ifndef __MACPORTS_ENDIAN_H
#define __MACPORTS_ENDIAN_H

#include <libkern/OSByteOrder.h>

/*
 * Macros for byte-swapping by operand size
 *
 * Although these use switch() statements, the switch() arg is a compile-time
 * constant, so the optimizer will normally reduce them to single cases.
 */

/* Common internal part */
#define __BYTE_SWAP_INTERNAL(dst,src,sizarg) \
    switch (sizeof(sizarg)) { \
    case 2: dst = OSSwapInt16(src); break; \
    case 4: dst = OSSwapInt32(src); break; \
    case 8: dst = OSSwapInt64(src); break; \
    }

/* In-place swap */
#define BYTE_SWAP_INPLACE(loc) __BYTE_SWAP_INTERNAL(loc,loc,loc)

/* Copy swapped by size of source */
#define BYTE_SWAP_BY_SOURCE(dst,src) __BYTE_SWAP_INTERNAL(dst,src,src)

/* Copy swapped by size of destination (source converted before swapping) */
#define BYTE_SWAP_BY_DEST(dst,src) \
    __BYTE_SWAP_INTERNAL(dst,(__typeof__(dst))(src),dst)

#endif /* __MACPORTS_ENDIAN_H */
