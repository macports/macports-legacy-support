/* Match the Leopard SDK behavior w.r.t. _DARWIN_C_SOURCE */
/*
 * Copyright (c) 2021 Evan Miller
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

#if defined(_POSIX_C_SOURCE) && defined(_DARWIN_C_SOURCE)
#define MACPORTS_OLD_POSIX_C_SOURCE _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#include_next <sys/termios.h>
#define _POSIX_C_SOURCE MACPORTS_OLD_POSIX_C_SOURCE
#undef MACPORTS_OLD_POSIX_C_SOURCE
#else
#include_next <sys/termios.h>
#endif /* _POSIX_C_SOURCE && _DARWIN_C_SOURCE */
