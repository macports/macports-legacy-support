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

#ifndef _MACPORTS_MACH_PPC__TYPES_H_
#define _MACPORTS_MACH_PPC__TYPES_H_
/*
 * This wrapper provides the _STRUCT_PPC_*_STATE macros for 10.4 ppc.
 *
 * It also provides macros to make the old 10.4 exception and register names
 * available under their 10.5+ names.  Since these are simple macros with short
 * names, there's some risk of collisions, though the leading underscores should
 * make that unlikely.  Just in case, we provide the flag
 * _MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES, which can be defined nonzero to
 * inhibit these definitions.
 *
 * Since this header only exists in the 10.4 SDK, there's no need for an
 * SDK version conditional for it.  Any attempt to use it with a 10.5+ SDK
 * will result in a failure of the include_next below.
 */

/* Include the primary system mach/ppc/_types.h (10.4 only) */
#include_next <mach/ppc/_types.h>

#ifndef _POSIX_C_SOURCE
#define _STRUCT_PPC_EXCEPTION_STATE struct ppc_exception_state
#define _STRUCT_PPC_THREAD_STATE struct ppc_thread_state
#define _STRUCT_PPC_FLOAT_STATE struct ppc_float_state
#define _STRUCT_PPC_VECTOR_STATE struct ppc_vector_state
#define _STRUCT_PPC_EXCEPTION_STATE64 struct ppc_exception_state64
#define _STRUCT_PPC_THREAD_STATE64 struct ppc_thread_state64
#else  /* _POSIX_C_SOURCE */
#define _STRUCT_PPC_EXCEPTION_STATE struct __darwin_ppc_exception_state
#define _STRUCT_PPC_THREAD_STATE struct __darwin_ppc_thread_state
#define _STRUCT_PPC_FLOAT_STATE struct __darwin_ppc_float_state
#define _STRUCT_PPC_VECTOR_STATE struct __darwin_ppc_vector_state
#define _STRUCT_PPC_EXCEPTION_STATE64 struct __darwin_ppc_exception_state64
#define _STRUCT_PPC_THREAD_STATE64 struct __darwin_ppc_thread_state64

/* Provide missing struct definitions */

struct __darwin_ppc_exception_state64 {
	unsigned long long dar;		/* Fault registers for coredump */
#if defined(__LP64__)
	unsigned int  dsisr;
	unsigned int  exception;	/* number of powerpc exception taken */
	unsigned int  pad1[4];		/* space in PCB "just in case" */
#else
	unsigned long dsisr;
	unsigned long exception;	/* number of powerpc exception taken */
	unsigned long pad1[4];		/* space in PCB "just in case" */
#endif
};

#pragma pack(4)							/* Make sure the structure stays as we defined it */
struct __darwin_ppc_thread_state64 {
	unsigned long long srr0;	/* Instruction address register (PC) */
	unsigned long long srr1;	/* Machine state register (supervisor) */
	unsigned long long r0;
	unsigned long long r1;
	unsigned long long r2;
	unsigned long long r3;
	unsigned long long r4;
	unsigned long long r5;
	unsigned long long r6;
	unsigned long long r7;
	unsigned long long r8;
	unsigned long long r9;
	unsigned long long r10;
	unsigned long long r11;
	unsigned long long r12;
	unsigned long long r13;
	unsigned long long r14;
	unsigned long long r15;
	unsigned long long r16;
	unsigned long long r17;
	unsigned long long r18;
	unsigned long long r19;
	unsigned long long r20;
	unsigned long long r21;
	unsigned long long r22;
	unsigned long long r23;
	unsigned long long r24;
	unsigned long long r25;
	unsigned long long r26;
	unsigned long long r27;
	unsigned long long r28;
	unsigned long long r29;
	unsigned long long r30;
	unsigned long long r31;

	unsigned int cr;			/* Condition register */
	unsigned long long xer;		/* User's integer exception register */
	unsigned long long lr;		/* Link register */
	unsigned long long ctr;		/* Count register */

	unsigned int vrsave;		/* Vector Save Register */
};
#pragma pack()

#endif  /* _POSIX_C_SOURCE */

#if !defined(_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES) \
    || !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES

/* Aliases for the item names in exception structures */

#define __dar dar
#define __dsisr dsisr
#define __exception exception

/* Aliases for the register names in context structures */

#define __srr0 srr0
#define __srr1 srr1
#define __r0 r0
#define __r1 r1
#define __r2 r2
#define __r3 r3
#define __r4 r4
#define __r5 r5
#define __r6 r6
#define __r7 r7
#define __r8 r8
#define __r9 r9
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __r16 r16
#define __r17 r17
#define __r18 r18
#define __r19 r19
#define __r20 r20
#define __r21 r21
#define __r22 r22
#define __r23 r23
#define __r24 r24
#define __r25 r25
#define __r26 r26
#define __r27 r27
#define __r28 r28
#define __r29 r29
#define __r30 r30
#define __r31 r31

#define __cr cr
#define __xer xer
#define __lr lr
#define __ctr ctr
/* No need to support mq (601 only) */

#define __vrsave vrsave

#endif  /* !_MACPORTS_LEGACY_DISABLE_CONTEXT_ALIASES */

#endif  /* _MACPORTS_MACH_PPC__TYPES_H_ */
