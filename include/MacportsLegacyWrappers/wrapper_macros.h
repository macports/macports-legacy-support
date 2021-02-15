
/*
 * Copyright (c) 2019 Christian Cornelssen
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

#ifndef _MACPORTS_LEGACYSUPPORTWRAP_H_
#define _MACPORTS_LEGACYSUPPORTWRAP_H_

/* We need support for __asm */
#if !__GNUC__ && !__clang__
#undef __DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__
#define __DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__ 1
#endif

#if !__DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__
/* Could include Darwin's <sys/cdefs.h> and use __STRING, __CONCAT */
/* But for wrappers we require __asm, thus GCC/Clang, thus ANSI C, anyway */

/* Wrapper support macros */
/* Use __asm instead of asm, as the latter is not recognized with e.g. -ansi */
#define __MP_LEGACY_WRAPPER(sym) macports_legacy_##sym
#define __MP_LEGACY_WRAPPER_ALIAS(sym) __asm("_macports_legacy_" #sym)

#endif /* !__DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__ */
#endif /* _MACPORTS_LEGACYSUPPORTWRAP_H_ */
