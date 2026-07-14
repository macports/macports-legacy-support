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

/*
 * This provides a version of syscall() that is valid for 64-bit return
 * values.  The standard Apple version works fine for all architectures
 * except i386, provided that a 64-bit version of the prototype is used.
 *
 * In the i386 ABI, the high-order part of 64-bit return values is supplied
 * in register edx, but the Apple syscall() wrapper clobbers this register
 * for an unknown reason.  So we provide an alternate implementation that
 * avoids clobbering edx.
 *
 * In the non-error case, this is quite straightforward.  But in the error
 * case, we need to store the eax return value in errno, which we do via
 * a separate C function.  In the illegal syscall number case, the kernel
 * has already generated SIGSYS.
 */

/* If i386 isn't even allowed in this OS, we don't need the dummy symbol */
#include <_macports_extras/targetos.h>
#define I386_ALLOWED (__MPLS_TARGET_OSVER < 101500)

#ifdef __i386__

/* Misc defs from kernel sources */

#ifdef __DYNAMIC__

#define NON_LAZY_STUB(var)  \
.section __IMPORT,__pointers,non_lazy_symbol_pointers ; \
L ## var ## __non_lazy_ptr: ; \
  .indirect_symbol var      ; \
  .long 0                   ; \
  .text

#define PICIFY(var) \
  call  1f      ; \
1:              ; \
  popl  %edx    ; \
  movl  L ## var ## __non_lazy_ptr-1b(%edx),%edx

#define BRANCH_EXTERN(func) \
  PICIFY(func) ; \
  jmp *%edx    ; \
  NON_LAZY_STUB(func)

#else  /* !__DYNAMIC__ */

#define BRANCH_EXTERN(func) jmp func

#endif  /* !__DYNAMIC__ */

/* Now the real function */

  .globl ___mpls_syscall64
  .align 2, 0x90
___mpls_syscall64:
  popl  %ecx    /* ret addr */
  popl  %eax    /* syscall number to eax */
  pushl %ecx    /* ret adr back on stack */
  /* Stack now has normal setup for remaining args; syscall # is in eax */
  int   $0x80   /* trap for the syscall */
  jb    2f      /* jump if carry flag set (error) */
  pushl %ecx    /* add one element to stack so caller "pop" will work */
  retl

2:
  popl  %ecx    /* ret addr */
  pushl %eax    /* errno value */
  pushl %ecx    /* ret addr */
  /* Stack now looks like original entry, but with err code as first arg */
  BRANCH_EXTERN(___mpls_set_errno)

#elif I386_ALLOWED && defined(__MPLS_UNIVERSAL__)  /* non-i386 slice */

/* Avoid "no symbols" warning from ranlib */

/* Make arm64 happy (when enabled, which this probably isn't) */
  .align 2

  .globl __mpls_empty_assembly
__mpls_empty_assembly:
  .long 0

#endif  /* non-i386 slice */
