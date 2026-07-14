/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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

/* If i386 isn't even allowed in this OS, we don't need the dummy symbol */
#include <_macports_extras/targetos.h>
#define I386_ALLOWED (__MPLS_TARGET_OSVER < 101500)

#if defined(__i386__)

#include <errno.h>
#include <stdint.h>

/* Internal error exit for syscall64() - set errno and return ~0 */

uint64_t
__mpls_set_errno(int err)
{
  errno = err;
  return ~0ULL;
}

#elif I386_ALLOWED && defined(__MPLS_UNIVERSAL__)  /* non-i386 slice */

/* Avoid "no symbols" warning from ranlib */
void __mpls_empty_assembly_support(void) {};

#endif  /* non-i386 slice */
