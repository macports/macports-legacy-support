/*
 * Copyright (c) 2026
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

#ifndef _MACPORTS_MACH_PORT_H_
#define _MACPORTS_MACH_PORT_H_

/*
 * This wrapper fixes a problem when compiling the SDK 26 headers with
 * an old compiler that doesn't support _Static_assert().  This header
 * relies on it for the new xnu_static_assert_struct_size(), which we define
 * as a dummy when _Static_assert() is unavailable.  Since this is conditioned
 * on the definition of xnu_static_assert_struct_size(), there's no need for
 * an SDK conditional.
 */

/* Include the primary system mach/port.h */
#include_next <mach/port.h>

#ifdef xnu_static_assert_struct_size
  #ifndef __has_extension
    #undef xnu_static_assert_struct_size
    #define xnu_static_assert_struct_size(a, b)
  #else
    #if !__has_extension(c_static_assert)
      #undef xnu_static_assert_struct_size
      #define xnu_static_assert_struct_size(a, b)
    #endif
  #endif
#endif

#endif /* _MACPORTS_MACH_PORT_H_ */
