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

#ifndef _MACPORTS_SYS_UCONTEXT_H_
#define _MACPORTS_SYS_UCONTEXT_H_
/*
 * This wrapper provides the definitions of _STRUCT_UCONTEXT[64] for 10.4.
 */

/* Include the primary system sys/ucontext.h */
#include_next <sys/ucontext.h>

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1050

#ifndef _POSIX_C_SOURCE
#define _STRUCT_UCONTEXT    struct ucontext
#define _STRUCT_UCONTEXT64  struct ucontext64
#else  /* _POSIX_C_SOURCE */
#define _STRUCT_UCONTEXT    struct __darwin_ucontext
#ifdef _DARWIN_C_SOURCE  /* Be like 10.5 */
#define _STRUCT_UCONTEXT64  struct __darwin_ucontext64
#endif  /* _DARWIN_C_SOURCE */
/* Provide missing struct definition */
struct __darwin_ucontext64 {
  int   uc_onstack;
  __darwin_sigset_t uc_sigmask; /* signal mask used by this context */
  __darwin_stack_t  uc_stack;   /* stack used by this context */
  struct __darwin_ucontext64 *uc_link;   /* pointer to resuming context */
  __darwin_size_t uc_mcsize;    /* size of the machine context passed in */
  __darwin_mcontext64_t uc_mcontext64;  /* pointer to machine specific context */
};
#endif  /* _POSIX_C_SOURCE */

#endif  /* __MPLS_SDK_MAJOR < 1050 */

#endif /* _MACPORTS_SYS_UCONTEXT_H_ */
