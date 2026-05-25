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

#ifndef _MACPORTS_MACH_I386_THREAD_STATUS_H_
#define _MACPORTS_MACH_I386_THREAD_STATUS_H_
/*
 * This wrapper provides the _STRUCT_X86_*_STATE macros for 10.4 x86.
 *
 * It also provides macros to make the old 10.4 exception and register names
 * available under their 10.5+ names.  Since these are simple macros with short
 * names, there's some risk of collisions, though the leading underscores should
 * make that unlikely.  Just in case, we provide the flag
 * _MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES, which can be defined nonzero to
 * inhibit these definitions.
 *
 * This header exists in all SDKs, so we need a 10.4 SDK conditional.
 */

/* Include the primary system mach/i386/thread_status.h */
#include_next <mach/i386/thread_status.h>

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1050

#define _STRUCT_X86_THREAD_STATE32 struct i386_thread_state
#define _STRUCT_X86_FLOAT_STATE32 struct i386_float_state
#define _STRUCT_X86_EXCEPTION_STATE32 struct i386_exception_state
#define _STRUCT_X86_THREAD_STATE64 struct x86_thread_state64
#define _STRUCT_X86_FLOAT_STATE64 struct x86_float_state64
#define _STRUCT_X86_EXCEPTION_STATE64 struct x86_exception_state64

#if !defined(_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES) \
    || !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES

/* Aliases for the item names in exception structures */

#define __trapno trapno
#define __err err
#define __faultvaddr faultvaddr

/* Aliases for the register names in context structures */

/* i386 */
#define __eax eax
#define __ebx ebx
#define __ecx ecx
#define __edx edx
#define __edi edi
#define __esi esi
#define __ebp ebp
#define __esp esp
#define __ss ss
#define __eflags eflags
#define __eip eip
#define __cs cs
#define __ds ds
#define __es es
#define __fs fs
#define __gs gs

/* x86_64 */
#define __rax rax
#define __rbx rbx
#define __rcx rcx
#define __rdx rdx
#define __rdi rdi
#define __rsi rsi
#define __rbp rbp
#define __rsp rsp
#define __r8 r8
#define __r9 r9
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __rip rip
#define __rflags rflags
#define __cs cs
#define __fs fs
#define __gs gs

#endif  /* !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES */

#endif  /* __MPLS_SDK_MAJOR < 1050 */

#endif /* _MACPORTS_MACH_I386_THREAD_STATUS_H_ */
