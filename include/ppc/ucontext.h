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

#ifndef _MACPORTS_PPC_UCONTEXT_H_
#define _MACPORTS_PPC_UCONTEXT_H_
/*
 * This wrapper provides the 10.5+ _STRUCT_MCONTEXT* macros for 10.4 ppc.
 *
 * It also provides macros to make the old 10.4 context names available
 * under their 10.5+ names.  Since these are simple macros with short names,
 * there's some risk of collisions, though the leading underscores should make
 * that unlikely.  Just in case, we provide the flag
 * _MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES, which can be defined nonzero to
 * inhibit these definitions.
 *
 * Since this header only exists in the 10.4 SDK, there's no need for an
 * SDK version conditional for it.  Any attempt to use it with a 10.5+ SDK
 * will result in a failure of the include_next below.
 */

/* Include the primary system ppc/ucontext.h (10.4 only) */
#include_next <ppc/ucontext.h>

#ifndef _POSIX_C_SOURCE
#define _STRUCT_MCONTEXT struct mcontext
#define _STRUCT_MCONTEXT64 struct mcontext64
#else  /* _POSIX_C_SOURCE */
#define _STRUCT_MCONTEXT struct __darwin_mcontext
#ifdef _DARWIN_C_SOURCE  /* Be like 10.5 */
#define _STRUCT_MCONTEXT64 struct __darwin_mcontext64
#endif  /* _DARWIN_C_SOURCE */
/* Provide missing struct definition */
struct __darwin_mcontext64 {
	struct __darwin_ppc_exception_state64	es;
	struct __darwin_ppc_thread_state64	ss;
	struct __darwin_ppc_float_state		fs;
	struct __darwin_ppc_vector_state	vs;
};
#endif  /* _POSIX_C_SOURCE */

#if !defined(_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES) \
    || !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES

/* Aliases for the structure elements for context structures */
#define __es es
#define __ss ss
#define __fs fs
#define __vs vs

#endif  /* !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES */

#endif  /* _MACPORTS_PPC_UCONTEXT_H_ */
