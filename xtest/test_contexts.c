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
 * This tests accesses to the context structures (including CPU registers),
 * to verify that the accesses work, even in 10.4.
 *
 * Because it is intended primarily to test the naming fixes for 10.4, it
 * doesn't concern itself with new context information added after 10.5,
 * and only the four CPUs from 10.4/10.5 are tested.
 *
 * In addition, the 10.4 signal trampoline code has a horrible bug in the
 * ppc64 case which causes crashes.  Since this test is intended to be used
 * without the library, we're at the mercy of the standard OS code.  However,
 * we also allow this test to be run as a manual test with the library.  So
 * we optionally disable the signal test in the problematic case, depending
 * on whether the library is present and advertises the fix.
 */

/* Get target OS version */
#include <_macports_extras/targetos.h>

#include <sys/cdefs.h>
/*
 * The $UNIX2003 versions of various calls are unavailable in 10.4, and
 * are supposed to be avoided when targeting 10.4.  Unfortunately, the
 * latter aspect doesn't work with some config-flag settings (which this
 * test needs), leading to link errors on 10.4 32-bit.  To avoid that, we
 * forcibly disable the suffix on 10.4, to avoid referencing things like
 * usleep$UNIX2003.
 */
#if __MPLS_TARGET_OSVER < 1050 && defined(__DARWIN_SUF_UNIX03)
#undef __DARWIN_SUF_UNIX03
#define __DARWIN_SUF_UNIX03
#endif

#include <dlfcn.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#include <sys/param.h>  /* For MIN */

/* Get the SDK version */
#include <_macports_extras/sdkversion.h>

/* What we're primarily testing */
#include <sys/ucontext.h>

/* Also test associated refs */
#include <setjmp.h>
#include <signal.h>

/* Set up broken-signal condition */
#ifdef __ppc64__
#define BROKEN_SIGNALS (__MPLS_TARGET_OSVER < 1050)
#else
#define BROKEN_SIGNALS 0
#endif

/* RTLD_NEXT is non-POSIX.  If necessary, use its "well-known" value. */
#ifndef RTLD_NEXT
#define	RTLD_NEXT		((void *) -1)	/* Search subsequent objects. */
#endif

/* Delay parameters (ms) */
#define DELAY_SETUP 20   /* Delay before setting up for signal */
#define DELAY_SIGNAL 20  /* Delay for signal in delay cases */

/* Use SIGVTALRM for setitimer(); also gdb doesn't intercept it by default */
#define TEST_SIG SIGVTALRM

#if defined(__ppc__)
#define CPU_TYPE ppc
#define CPU_PPCX
#define TESTED_CPU 1
#elif defined(__ppc64__)
#define CPU_TYPE ppc64
#define CPU_PPCX
#define TESTED_CPU 1
#elif defined(__i386__)
#define CPU_TYPE i386
#define CPU_X86
#define TESTED_CPU 1
#elif defined(__x86_64__)
#define CPU_TYPE x86_64
#define CPU_X86
#define TESTED_CPU 1
#elif defined(__arm__)
#define CPU_TYPE arm
#define CPU_ARMX
#define TESTED_CPU 1
#elif defined(__arm64__)
#define CPU_TYPE arm64
#define CPU_ARMX
#define TESTED_CPU 1
#else
#define CPU_TYPE <unknown>
#define TESTED_CPU 0
#endif

#define EXPAND(x) #x
#define EXPAND2(x) EXPAND(x)
#define PRINT_VAL(x) printf("  " #x " = %lld\n", (long long) x)
#define PRINT_TEXT(x) printf("  " #x " = \"%s\"\n", EXPAND(x))
#define PRINT_SIZE(x) printf("    sizeof(" #x ") = %zd\n", sizeof(x));
#define PRINT_UNDEF(x) printf("  " #x " is undefined\n")

/* Config-dependent name prefix */
#if __DARWIN_UNIX03
#define __(x) __ ## x
#define STR__(x) "__" #x
#else
#define __(x) x
#define STR__(x) #x
#endif

#ifdef __LP64__
typedef unsigned long long pointer_int_t;
#define PTR_FMT "%016llx"
#else
typedef unsigned int pointer_int_t;
#define PTR_FMT "%08X"
#endif

#define SS_FMT "%08X"

/* Types for CPU registers */
typedef unsigned int register32_t;
typedef unsigned long long register64_t;

/* Macros for CPU register sets */

#define PPCX_REGISTERS(rtype) \
  CPU_REG(rtype,srr0,srr0) \
  CPU_REG(rtype,srr1,srr1) \
  CPU_REG(rtype,r0,r0) \
  CPU_REG(rtype,r1,r1) \
  CPU_REG(rtype,r2,r2) \
  CPU_REG(rtype,r3,r3) \
  CPU_REG(rtype,r4,r4) \
  CPU_REG(rtype,r5,r5) \
  CPU_REG(rtype,r6,r6) \
  CPU_REG(rtype,r7,r7) \
  CPU_REG(rtype,r8,r8) \
  CPU_REG(rtype,r9,r9) \
  CPU_REG(rtype,r10,r10) \
  CPU_REG(rtype,r11,r11) \
  CPU_REG(rtype,r12,r12) \
  CPU_REG(rtype,r13,r13) \
  CPU_REG(rtype,r14,r14) \
  CPU_REG(rtype,r15,r15) \
  CPU_REG(rtype,r16,r16) \
  CPU_REG(rtype,r17,r17) \
  CPU_REG(rtype,r18,r18) \
  CPU_REG(rtype,r19,r19) \
  CPU_REG(rtype,r20,r20) \
  CPU_REG(rtype,r21,r21) \
  CPU_REG(rtype,r22,r22) \
  CPU_REG(rtype,r23,r23) \
  CPU_REG(rtype,r24,r24) \
  CPU_REG(rtype,r25,r25) \
  CPU_REG(rtype,r26,r26) \
  CPU_REG(rtype,r27,r27) \
  CPU_REG(rtype,r28,r28) \
  CPU_REG(rtype,r29,r29) \
  CPU_REG(rtype,r30,r30) \
  CPU_REG(rtype,r31,r31) \
  CPU_REG(register32_t,cr,cr) \
  CPU_REG(rtype,xer,xer) \
  CPU_REG(rtype,lr,lr) \
  CPU_REG(rtype,ctr,ctr) \
  CPU_REG(register32_t,vrsave,vrsave) \

#define PPC_REGISTERS PPCX_REGISTERS(register32_t)
#define PPC64_REGISTERS PPCX_REGISTERS(register64_t)

#define I386_REGISTERS \
  CPU_REG(register32_t,eax,eax) \
  CPU_REG(register32_t,ebx,ebx) \
  CPU_REG(register32_t,ecx,ecx) \
  CPU_REG(register32_t,edx,edx) \
  CPU_REG(register32_t,edi,edi) \
  CPU_REG(register32_t,esi,esi) \
  CPU_REG(register32_t,ebp,ebp) \
  CPU_REG(register32_t,esp,esp) \
  CPU_REG(register32_t,ss,ss) \
  CPU_REG(register32_t,eflags,eflags) \
  CPU_REG(register32_t,eip,eip) \
  CPU_REG(register32_t,cs,cs) \
  CPU_REG(register32_t,ds,ds) \
  CPU_REG(register32_t,es,es) \
  CPU_REG(register32_t,fs,fs) \
  CPU_REG(register32_t,gs,gs) \

#define X86_64_REGISTERS \
  CPU_REG(register64_t,rax,rax) \
  CPU_REG(register64_t,rbx,rbx) \
  CPU_REG(register64_t,rcx,rcx) \
  CPU_REG(register64_t,rdx,rdx) \
  CPU_REG(register64_t,rdi,rdi) \
  CPU_REG(register64_t,rsi,rsi) \
  CPU_REG(register64_t,rbp,rbp) \
  CPU_REG(register64_t,rsp,rsp) \
  CPU_REG(register64_t,r8,r8) \
  CPU_REG(register64_t,r9,r9) \
  CPU_REG(register64_t,r10,r10) \
  CPU_REG(register64_t,r11,r11) \
  CPU_REG(register64_t,r12,r12) \
  CPU_REG(register64_t,r13,r13) \
  CPU_REG(register64_t,r14,r14) \
  CPU_REG(register64_t,r15,r15) \
  CPU_REG(register64_t,rip,rip) \
  CPU_REG(register64_t,rflags,rflags) \
  CPU_REG(register64_t,cs,cs) \
  CPU_REG(register64_t,fs,fs) \
  CPU_REG(register64_t,gs,gs) \

#define ARM_REGISTERS \
  CPU_REG(register32_t,r0,r[0]) \
  CPU_REG(register32_t,r1,r[1]) \
  CPU_REG(register32_t,r2,r[2]) \
  CPU_REG(register32_t,r3,r[3]) \
  CPU_REG(register32_t,r4,r[4]) \
  CPU_REG(register32_t,r5,r[5]) \
  CPU_REG(register32_t,r6,r[6]) \
  CPU_REG(register32_t,r7,r[7]) \
  CPU_REG(register32_t,r8,r[8]) \
  CPU_REG(register32_t,r9,r[9]) \
  CPU_REG(register32_t,r10,r[10]) \
  CPU_REG(register32_t,r11,r[11]) \
  CPU_REG(register32_t,r12,r[12]) \
  CPU_REG(register32_t,sp,sp) \
  CPU_REG(register32_t,lr,lr) \
  CPU_REG(register32_t,pc,pc) \
  CPU_REG(register32_t,cpsr,cpsr) \

#define ARM64_REGISTERS \
  CPU_REG(register64_t,x0,x[0]) \
  CPU_REG(register64_t,x1,x[1]) \
  CPU_REG(register64_t,x2,x[2]) \
  CPU_REG(register64_t,x3,x[3]) \
  CPU_REG(register64_t,x4,x[4]) \
  CPU_REG(register64_t,x5,x[5]) \
  CPU_REG(register64_t,x6,x[6]) \
  CPU_REG(register64_t,x7,x[7]) \
  CPU_REG(register64_t,x8,x[8]) \
  CPU_REG(register64_t,x9,x[9]) \
  CPU_REG(register64_t,x10,x[10]) \
  CPU_REG(register64_t,x11,x[11]) \
  CPU_REG(register64_t,x12,x[12]) \
  CPU_REG(register64_t,x13,x[13]) \
  CPU_REG(register64_t,x14,x[14]) \
  CPU_REG(register64_t,x15,x[15]) \
  CPU_REG(register64_t,x16,x[16]) \
  CPU_REG(register64_t,x17,x[17]) \
  CPU_REG(register64_t,x18,x[18]) \
  CPU_REG(register64_t,x19,x[19]) \
  CPU_REG(register64_t,x20,x[20]) \
  CPU_REG(register64_t,x21,x[21]) \
  CPU_REG(register64_t,x22,x[22]) \
  CPU_REG(register64_t,x23,x[23]) \
  CPU_REG(register64_t,x24,x[24]) \
  CPU_REG(register64_t,x25,x[25]) \
  CPU_REG(register64_t,x26,x[26]) \
  CPU_REG(register64_t,x27,x[27]) \
  CPU_REG(register64_t,x28,x[28]) \
  CPU_REG(register64_t,fp,fp) \
  CPU_REG(register64_t,lr,lr) \
  CPU_REG(register64_t,sp,sp) \
  CPU_REG(register64_t,pc,pc) \
  CPU_REG(register32_t,cpsr,cpsr) \

#if defined(CPU_PPCX)

#define CPU_REGISTERS32 PPC_REGISTERS
#define CPU_REGISTERS64 PPC64_REGISTERS

#if defined(__ppc__)
#define CPU_REGISTERS PPC_REGISTERS
#elif defined(__ppc64__)
#define CPU_REGISTERS PPC64_REGISTERS
#endif

#elif defined(CPU_X86)

#define CPU_REGISTERS32 I386_REGISTERS
#define CPU_REGISTERS64 X86_64_REGISTERS

#if defined(__i386__)
#define CPU_REGISTERS I386_REGISTERS
#elif defined(__x86_64)
#define CPU_REGISTERS X86_64_REGISTERS
#endif

#elif defined(CPU_ARMX)

#define CPU_REGISTERS32 ARM_REGISTERS
#define CPU_REGISTERS64 ARM64_REGISTERS

#if defined(__arm__)
#define CPU_REGISTERS ARM_REGISTERS
#elif defined(__arm64__)
#define CPU_REGISTERS ARM64_REGISTERS
#endif

#endif  /* All CPUs */

/* Instances of all the relevant structures */

#if defined(CPU_PPCX)

_STRUCT_PPC_EXCEPTION_STATE xxx_exception32;
_STRUCT_PPC_THREAD_STATE xxx_thread32;
_STRUCT_PPC_FLOAT_STATE xxx_float32;
_STRUCT_PPC_VECTOR_STATE xxx_vector32;
#define HAVE_FLOAT32
#define HAVE_VECTOR32

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
_STRUCT_PPC_EXCEPTION_STATE64 xxx_exception64;
_STRUCT_PPC_THREAD_STATE64 xxx_thread64;
#define HAVE_THREAD64
#endif  /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

_STRUCT_PPC_EXCEPTION_STATE xxx_exception;
_STRUCT_PPC_THREAD_STATE xxx_thread;

size_t xxx_mcontext_size;
_STRUCT_MCONTEXT xxx_mcontext;
size_t xxx_mcontext32_size;
_STRUCT_MCONTEXT xxx_mcontext32;
#define _STRUCT_MCONTEXT32 _STRUCT_MCONTEXT

#ifdef _STRUCT_MCONTEXT64
size_t xxx_mcontext64_size;
_STRUCT_MCONTEXT64 xxx_mcontext64;
#endif  /* _STRUCT_MCONTEXT64 */

#endif  /* CPU_PPCX */

#if defined(CPU_X86)

_STRUCT_X86_EXCEPTION_STATE32 xxx_exception32;
_STRUCT_X86_THREAD_STATE32 xxx_thread32;
_STRUCT_X86_FLOAT_STATE32 xxx_float32;
#define HAVE_FLOAT32

_STRUCT_X86_EXCEPTION_STATE64 xxx_exception64;
_STRUCT_X86_THREAD_STATE64 xxx_thread64;
_STRUCT_X86_FLOAT_STATE64 xxx_float64;
#define HAVE_THREAD64
#define HAVE_FLOAT64

#ifdef __LP64__
_STRUCT_X86_EXCEPTION_STATE64 xxx_exception;
_STRUCT_X86_THREAD_STATE64 xxx_thread;
#else  /* !__LP64__ */
_STRUCT_X86_EXCEPTION_STATE32 xxx_exception;
_STRUCT_X86_THREAD_STATE32 xxx_thread;
#endif  /* !__LP64__ */

#ifdef _STRUCT_MCONTEXT
size_t xxx_mcontext_size;
_STRUCT_MCONTEXT xxx_mcontext;
#endif  /* _STRUCT_MCONTEXT */

#ifdef _STRUCT_MCONTEXT32
size_t xxx_mcontext32_size;
_STRUCT_MCONTEXT32 xxx_mcontext32;
#endif  /* _STRUCT_MCONTEXT32 */

#ifdef _STRUCT_MCONTEXT64
size_t xxx_mcontext64_size;
_STRUCT_MCONTEXT64 xxx_mcontext64;
#endif  /* _STRUCT_MCONTEXT64 */

#endif  /* CPU_X86 */

#if defined(CPU_ARMX)

_STRUCT_ARM_EXCEPTION_STATE xxx_exception32;
_STRUCT_ARM_THREAD_STATE xxx_thread32;

_STRUCT_ARM_EXCEPTION_STATE64 xxx_exception64;
_STRUCT_ARM_THREAD_STATE64 xxx_thread64;
#define HAVE_THREAD64

#ifdef __LP64__
_STRUCT_ARM_EXCEPTION_STATE64 xxx_exception;
_STRUCT_ARM_THREAD_STATE64 xxx_thread;
#else  /* !__LP64__ */
_STRUCT_ARM_EXCEPTION_STATE xxx_exception;
_STRUCT_ARM_THREAD_STATE xxx_thread;
#endif  /* !__LP64__ */

#ifdef _STRUCT_MCONTEXT
size_t xxx_mcontext_size;
_STRUCT_MCONTEXT xxx_mcontext;
#endif  /* _STRUCT_MCONTEXT */

#ifdef _STRUCT_MCONTEXT32
size_t xxx_mcontext32_size;
_STRUCT_MCONTEXT32 xxx_mcontext32;
#endif  /* _STRUCT_MCONTEXT32 */

#ifdef _STRUCT_MCONTEXT64
size_t xxx_mcontext64_size;
_STRUCT_MCONTEXT64 xxx_mcontext64;
#endif  /* _STRUCT_MCONTEXT64 */

#endif  /* CPU_ARMX */

#ifdef _STRUCT_UCONTEXT
_STRUCT_UCONTEXT *xxx_ucontextp, xxx_ucontext;
#endif  /* _STRUCT_UCONTEXT */

/* _STRUCT_UCONTEXT64 is only for ppc* ~POSIX */
#ifdef _STRUCT_UCONTEXT64
_STRUCT_UCONTEXT64 *xxx_ucontext64p, xxx_ucontext64;
#endif  /* _STRUCT_UCONTEXT64 */

#if !defined(_STRUCT_MCONTEXT32) && !defined(_STRUCT_MCONTEXT64) && TESTED_CPU
#error no _STRUCT_MCONTEXTxx definition
#endif

#if !defined(_STRUCT_UCONTEXT) && !defined(_STRUCT_UCONTEXT64) && TESTED_CPU
#error no _STRUCT_UCONTEXTxx definition
#endif

#if TESTED_CPU

/* Copy contexts to check names and types */
static void
copy_contexts(void)
{
  #ifdef _STRUCT_MCONTEXT
  xxx_mcontext.__(es) = xxx_exception;
  xxx_mcontext.__(ss) = xxx_thread;
  #endif  /* _STRUCT_MCONTEXT */

  #ifdef _STRUCT_MCONTEXT32
  xxx_mcontext32.__(es) = xxx_exception32;
  xxx_mcontext32.__(ss) = xxx_thread32;
  #ifdef HAVE_FLOAT32
  xxx_mcontext32.__(fs) = xxx_float32;
  #endif
  #ifdef HAVE_VECTOR32
  xxx_mcontext32.__(vs) = xxx_vector32;
  #endif
  #endif  /* _STRUCT_MCONTEXT32 */

  #ifdef _STRUCT_MCONTEXT64
  xxx_mcontext64.__(es) = xxx_exception64;
  xxx_mcontext64.__(ss) = xxx_thread64;
  #ifdef HAVE_FLOAT64
  xxx_mcontext64.__(fs) = xxx_float64;
  #endif
  #endif  /* _STRUCT_MCONTEXT64 */

  #if defined(_STRUCT_UCONTEXT)
  xxx_ucontext.uc_mcontext = &xxx_mcontext;
  #endif  /* _STRUCT_UCONTEXT */

  #ifdef _STRUCT_UCONTEXT64
  xxx_ucontext64.uc_mcontext64 = &xxx_mcontext64;
  #endif  /* _STRUCT_UCONTEXT64 */
}

/* Copy register pointers to check names and types */
static void
copy_regs(void)
{
  #ifdef CPU_REGISTERS32
  #define CPU_REG(rtype,name,ref) \
    rtype *reg32_ ## name = &xxx_thread32.__(ref); (void) reg32_ ## name;
  CPU_REGISTERS32
  #undef CPU_REG
  #endif

  #if defined(CPU_REGISTERS64) && defined(HAVE_THREAD64)
  #define CPU_REG(rtype,name,ref) \
    rtype *reg64_ ## name = &xxx_thread64.__(ref); (void) reg64_ ## name;
  CPU_REGISTERS64
  #undef CPU_REG
  #endif
}

/* Check context info provided by signal handler */

/* Structure for result flags */
typedef struct sigcheck_s {
  volatile int done;
  int have_ucontext;
  int have_ucontext64;
  int have_mcontext;
  int have_mcontext64;
} sigcheck_t;

static sigcheck_t sig_check, sig_clear = {0};

/* Signal handler - just copies contexts for later perusal */
static void
sig_handler(int sig, siginfo_t *info, void *uap)
{
  size_t csize;
#ifdef _STRUCT_UCONTEXT
  _STRUCT_UCONTEXT *ucp = uap;
#endif
#ifdef _STRUCT_UCONTEXT64
  _STRUCT_UCONTEXT64 *uc64p = uap;
#endif

#ifdef _STRUCT_UCONTEXT
  xxx_ucontextp = ucp;
  xxx_ucontext = *ucp;
  sig_check.have_ucontext = 1;
#ifdef _STRUCT_MCONTEXT
  xxx_mcontext_size = ucp->uc_mcsize;
  if (ucp->uc_mcontext && ucp->uc_mcsize) {
    csize = MIN(ucp->uc_mcsize, sizeof(xxx_mcontext));
    memcpy(&xxx_mcontext, ucp->uc_mcontext, csize);
    sig_check.have_mcontext = 1;
  }
#endif
#endif

#ifdef _STRUCT_UCONTEXT64
  xxx_ucontext64p = uc64p;
  xxx_ucontext64 = *uc64p;
  sig_check.have_ucontext64 = 1;
#ifdef _STRUCT_MCONTEXT64
  xxx_mcontext64_size = uc64p->uc_mcsize;
  if (uc64p->uc_mcontext64 && uc64p->uc_mcsize) {
    csize = MIN(uc64p->uc_mcsize, sizeof(xxx_mcontext64));
    memcpy(&xxx_mcontext64, uc64p->uc_mcontext64, csize);
    sig_check.have_mcontext64 = 1;
  }
#endif
#endif

  sig_check.done = 1;
}

/* Get context via signal */
static int
get_sigcontext()
{
  int err;
  struct sigaction act, oact;
  struct itimerval itv;
  static struct timeval tv_zero = {0, 0};
  static struct timeval tv_delay = {0, DELAY_SIGNAL * 1000};

  act.sa_sigaction = sig_handler;
  act.sa_mask = 0;
  act.sa_flags = SA_SIGINFO;

  sig_check = sig_clear;

  if (sigaction(TEST_SIG, &act, &oact)) {
    perror("sigaction() failed");
    return -1;
  }

  /* Get a fresh quantum so we don't prematurely reschedule */
  (void) usleep(DELAY_SETUP * 1000);

  do {
    itv.it_interval = tv_zero;
    itv.it_value = tv_delay;
    err = setitimer(ITIMER_VIRTUAL, &itv, NULL);
    if (err) {
      perror("setitimer() for signal failed");
      break;
    }
    /* Wait for signal to happen */
    while (!sig_check.done) ;
  } while (0);
  if (signal(TEST_SIG, SIG_DFL) == SIG_ERR) {
    perror("signal() to remove handler failed");
    return -1;
  }
  if (err) return -1;

  return 0;
}

static int
check_contexts(void)
{
  if (!(sig_check.have_ucontext || sig_check.have_ucontext64)) {
    printf("No ucontext[64] provided by signal handler\n");
    return 1;
  }
  if (!(sig_check.have_mcontext || sig_check.have_mcontext64)) {
    printf("No mcontext[64] provided by signal handler\n");
    return 1;
  }
  return 0;
}

#else  /* !TESTED_CPU */

static void copy_contexts(void) {}
static void copy_regs(void) {}
static int get_sigcontext(void) { return 0; }
static int check_contexts(void) { return 0; }

#endif  /* !TESTED_CPU */

/* Print various interesting macro definitions */

static void
print_macros(void)
{
  printf("\n");

  PRINT_VAL(__MPLS_TARGET_OSVER);
  PRINT_TEXT(CPU_TYPE);
  PRINT_VAL(__MPLS_SDK_MAJOR);
  #ifdef _POSIX_C_SOURCE
  PRINT_VAL(_POSIX_C_SOURCE);
  #else
  PRINT_UNDEF(_POSIX_C_SOURCE);
  #endif
  #ifdef _DARWIN_C_SOURCE
  PRINT_VAL(_DARWIN_C_SOURCE);
  #else
  PRINT_UNDEF(_DARWIN_C_SOURCE);
  #endif
  #ifdef _XOPEN_SOURCE
  PRINT_VAL(_XOPEN_SOURCE);
  #else
  PRINT_UNDEF(_XOPEN_SOURCE);
  #endif
  #ifdef __DARWIN_UNIX03
  PRINT_VAL(__DARWIN_UNIX03);
  #else
  PRINT_UNDEF(__DARWIN_UNIX03);
  #endif
  #ifdef __DARWIN_SUF_UNIX03_SET
  PRINT_VAL(__DARWIN_SUF_UNIX03_SET);
  #else
  PRINT_UNDEF(__DARWIN_SUF_UNIX03_SET);
  #endif
  printf("\n");

  #ifdef _STRUCT_SIGCONTEXT
  PRINT_TEXT(_STRUCT_SIGCONTEXT);
  PRINT_SIZE(_STRUCT_SIGCONTEXT);
  #else
  PRINT_UNDEF(_STRUCT_SIGCONTEXT);
  #endif
  #ifdef _STRUCT_SIGCONTEXT32
  PRINT_TEXT(_STRUCT_SIGCONTEXT32);
  PRINT_SIZE(_STRUCT_SIGCONTEXT32);
  #else
  PRINT_UNDEF(_STRUCT_SIGCONTEXT32);
  #endif
  #ifdef _STRUCT_SIGCONTEXT64
  PRINT_TEXT(_STRUCT_SIGCONTEXT64);
  PRINT_SIZE(_STRUCT_SIGCONTEXT64);
  #else
  PRINT_UNDEF(_STRUCT_SIGCONTEXT64);
  #endif
  printf("\n");

  #ifdef _STRUCT_UCONTEXT
  PRINT_TEXT(_STRUCT_UCONTEXT);
  PRINT_SIZE(_STRUCT_UCONTEXT);
  #else
  PRINT_UNDEF(_STRUCT_UCONTEXT);
  #endif
  #ifdef _STRUCT_UCONTEXT64
  PRINT_TEXT(_STRUCT_UCONTEXT64);
  PRINT_SIZE(_STRUCT_UCONTEXT64);
  #else
  PRINT_UNDEF(_STRUCT_UCONTEXT64);
  #endif
  printf("\n");

  #ifdef _STRUCT_MCONTEXT
  PRINT_TEXT(_STRUCT_MCONTEXT);
  PRINT_SIZE(_STRUCT_MCONTEXT);
  #else
  PRINT_UNDEF(_STRUCT_MCONTEXT);
  #endif
  #ifdef _STRUCT_MCONTEXT32
  PRINT_TEXT(_STRUCT_MCONTEXT32);
  PRINT_SIZE(_STRUCT_MCONTEXT32);
  #else
  PRINT_UNDEF(_STRUCT_MCONTEXT32);
  #endif
  #ifdef _STRUCT_MCONTEXT64
  PRINT_TEXT(_STRUCT_MCONTEXT64);
  PRINT_SIZE(_STRUCT_MCONTEXT64);
  #else
  PRINT_UNDEF(_STRUCT_MCONTEXT64);
  #endif
  printf("\n");

  #ifdef _STRUCT_PPC_EXCEPTION_STATE
  PRINT_TEXT(_STRUCT_PPC_EXCEPTION_STATE);
  PRINT_SIZE(_STRUCT_PPC_EXCEPTION_STATE);
  #else
  PRINT_UNDEF(_STRUCT_PPC_EXCEPTION_STATE);
  #endif
  #ifdef _STRUCT_PPC_THREAD_STATE
  PRINT_TEXT(_STRUCT_PPC_THREAD_STATE);
  PRINT_SIZE(_STRUCT_PPC_THREAD_STATE);
  #else
  PRINT_UNDEF(_STRUCT_PPC_THREAD_STATE);
  #endif
  #ifdef _STRUCT_PPC_FLOAT_STATE
  PRINT_TEXT(_STRUCT_PPC_FLOAT_STATE);
  PRINT_SIZE(_STRUCT_PPC_FLOAT_STATE);
  #else
  PRINT_UNDEF(_STRUCT_PPC_FLOAT_STATE);
  #endif
  #ifdef _STRUCT_PPC_VECTOR_STATE
  PRINT_TEXT(_STRUCT_PPC_VECTOR_STATE);
  PRINT_SIZE(_STRUCT_PPC_VECTOR_STATE);
  #else
  PRINT_UNDEF(_STRUCT_PPC_VECTOR_STATE);
  #endif
  printf("\n");

  #ifdef _STRUCT_PPC_EXCEPTION_STATE64
  PRINT_TEXT(_STRUCT_PPC_EXCEPTION_STATE64);
  PRINT_SIZE(_STRUCT_PPC_EXCEPTION_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_PPC_EXCEPTION_STATE64);
  #endif
  #ifdef _STRUCT_PPC_THREAD_STATE64
  PRINT_TEXT(_STRUCT_PPC_THREAD_STATE64);
  PRINT_SIZE(_STRUCT_PPC_THREAD_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_PPC_THREAD_STATE64);
  #endif
  printf("\n");

  #ifdef _STRUCT_X86_EXCEPTION_STATE32
  PRINT_TEXT(_STRUCT_X86_EXCEPTION_STATE32);
  PRINT_SIZE(_STRUCT_X86_EXCEPTION_STATE32);
  #else
  PRINT_UNDEF(_STRUCT_X86_EXCEPTION_STATE32);
  #endif
  #ifdef _STRUCT_X86_THREAD_STATE32
  PRINT_TEXT(_STRUCT_X86_THREAD_STATE32);
  PRINT_SIZE(_STRUCT_X86_THREAD_STATE32);
  #else
  PRINT_UNDEF(_STRUCT_X86_THREAD_STATE32);
  #endif
  #ifdef _STRUCT_X86_FLOAT_STATE32
  PRINT_TEXT(_STRUCT_X86_FLOAT_STATE32);
  PRINT_SIZE(_STRUCT_X86_FLOAT_STATE32);
  #else
  PRINT_UNDEF(_STRUCT_X86_FLOAT_STATE32);
  #endif
  printf("\n");

  #ifdef _STRUCT_X86_EXCEPTION_STATE64
  PRINT_TEXT(_STRUCT_X86_EXCEPTION_STATE64);
  PRINT_SIZE(_STRUCT_X86_EXCEPTION_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_X86_EXCEPTION_STATE64);
  #endif
  #ifdef _STRUCT_X86_THREAD_STATE64
  PRINT_TEXT(_STRUCT_X86_THREAD_STATE64);
  PRINT_SIZE(_STRUCT_X86_THREAD_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_X86_THREAD_STATE64);
  #endif
  #ifdef _STRUCT_X86_FLOAT_STATE64
  PRINT_TEXT(_STRUCT_X86_FLOAT_STATE64);
  PRINT_SIZE(_STRUCT_X86_FLOAT_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_X86_FLOAT_STATE64);
  #endif
  printf("\n");

  #ifdef _STRUCT_ARM_EXCEPTION_STATE
  PRINT_TEXT(_STRUCT_ARM_EXCEPTION_STATE);
  PRINT_SIZE(_STRUCT_ARM_EXCEPTION_STATE);
  #else
  PRINT_UNDEF(_STRUCT_ARM_EXCEPTION_STATE);
  #endif
  #ifdef _STRUCT_ARM_THREAD_STATE
  PRINT_TEXT(_STRUCT_ARM_THREAD_STATE);
  PRINT_SIZE(_STRUCT_ARM_THREAD_STATE);
  #else
  PRINT_UNDEF(_STRUCT_ARM_THREAD_STATE);
  #endif
  printf("\n");

  #ifdef _STRUCT_ARM_EXCEPTION_STATE64
  PRINT_TEXT(_STRUCT_ARM_EXCEPTION_STATE64);
  PRINT_SIZE(_STRUCT_ARM_EXCEPTION_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_ARM_EXCEPTION_STATE64);
  #endif
  #ifdef _STRUCT_ARM_THREAD_STATE64
  PRINT_TEXT(_STRUCT_ARM_THREAD_STATE64);
  PRINT_SIZE(_STRUCT_ARM_THREAD_STATE64);
  #else
  PRINT_UNDEF(_STRUCT_ARM_THREAD_STATE64);
  #endif
  printf("\n");
}

#if TESTED_CPU

/* Print context information captured by signal handler */

#if defined(CPU_PPCX)

#define PRINT_EXCEPTION(estr) \
  printf("    " STR__(dar) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(dar)); \
  printf("    " STR__(dsisr) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(dsisr)); \
  printf("    " STR__(exception) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(exception)); \

#define PRINT_EXCEPTION64(estr) PRINT_EXCEPTION(estr)

#elif defined(CPU_X86)

#define PRINT_EXCEPTION(estr) \
  printf("    " STR__(trapno) " = %u\n", \
         estr.__(es).__(trapno)); \
  printf("    " STR__(err) " = 0x%X\n", \
         estr.__(es).__(err)); \
  printf("    " STR__(faultvaddr) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(faultvaddr)); \

#define PRINT_EXCEPTION64(estr) PRINT_EXCEPTION(estr)

#elif defined(CPU_ARMX)

#define PRINT_EXCEPTION32(estr) \
  printf("    " STR__(exception) " = 0x%X\n", \
         estr.__(es).__(exception)); \
  printf("    " STR__(fsr) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(fsr)); \
  printf("    " STR__(far) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(far)); \

#define PRINT_EXCEPTION64_V1(estr) \
  printf("    " STR__(far) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(far)); \
  printf("    " STR__(esr) " = 0x%X\n", \
         estr.__(es).__(esr)); \
  printf("    " STR__(exception) " = 0x%X\n", \
         estr.__(es).__(exception)); \

#define PRINT_EXCEPTION64_V2(estr) \
  printf("    " STR__(far) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(far)); \
  printf("    " STR__(esr) " = 0x" PTR_FMT "\n", \
         (pointer_int_t) estr.__(es).__(esr)); \

#define PRINT_EXCEPTION64(estr) PRINT_EXCEPTION64_V2(estr)

#ifdef __LP64__
#define PRINT_EXCEPTION(estr) PRINT_EXCEPTION64(estr)
#else
#define PRINT_EXCEPTION(estr) PRINT_EXCEPTION32(estr)
#endif

#endif  /* All CPUs */

static void
print_contexts(void)
{
#ifdef _STRUCT_UCONTEXT
  if (sig_check.have_ucontext) {
    printf("struct ucontext @ 0x" PTR_FMT ":\n",
           (pointer_int_t) xxx_ucontextp);
    printf("  uc_onstack = %d\n", xxx_ucontext.uc_onstack);
    printf("  uc_sigmask = 0x" SS_FMT "\n", xxx_ucontext.uc_sigmask);
    printf("  uc_link = 0x" PTR_FMT "\n",
           (pointer_int_t) xxx_ucontext.uc_link);
    printf("  uc_mcsize = %zd\n", xxx_ucontext.uc_mcsize);
    printf("  uc_mcontext = 0x" PTR_FMT "\n",
           (pointer_int_t) xxx_ucontext.uc_mcontext);
    printf("\n");
  }
#endif

#ifdef _STRUCT_MCONTEXT
  if (sig_check.have_mcontext) {
    printf("struct mcontext:\n");
    if (xxx_mcontext_size >= sizeof(xxx_mcontext.__(es))) {
      printf("  " STR__(es) ":\n");
      PRINT_EXCEPTION(xxx_mcontext);
    }
    if (xxx_mcontext_size >=
        sizeof(xxx_mcontext.__(es)) + sizeof(xxx_mcontext.__(ss))) {
      printf("  " STR__(ss) ":\n");
      #define CPU_REG(rtype,name,ref) \
        printf("    " STR__(name) " = 0x%0*llX\n", \
               (int) sizeof(rtype) * 2, \
               (unsigned long long) xxx_mcontext.__(ss).__(ref));
      CPU_REGISTERS
      #undef CPU_REG
    }
    if (xxx_mcontext_size >
        sizeof(xxx_mcontext.__(es)) + sizeof(xxx_mcontext.__(ss))) {
      printf("  [...]\n");
    }
    printf("\n");
  }
#endif

#ifdef _STRUCT_UCONTEXT64
  if (sig_check.have_ucontext64) {
    printf("struct ucontext64 @ 0x" PTR_FMT ":\n",
           (pointer_int_t) xxx_ucontext64p);
    printf("  uc_onstack = %d\n", xxx_ucontext64.uc_onstack);
    printf("  uc_sigmask = 0x" SS_FMT "\n", xxx_ucontext64.uc_sigmask);
    printf("  uc_link = 0x" PTR_FMT "\n",
           (pointer_int_t) xxx_ucontext64.uc_link);
    printf("  uc_mcsize = %zd\n", xxx_ucontext64.uc_mcsize);
    printf("  uc_mcontext64 = 0x" PTR_FMT "\n",
           (pointer_int_t) xxx_ucontext64.uc_mcontext64);
    printf("\n");
  }
#endif

#ifdef _STRUCT_MCONTEXT64
  if (sig_check.have_mcontext64) {
    printf("struct mcontext64:\n");
    if (xxx_mcontext64_size >= sizeof(xxx_mcontext64.__(es))) {
      printf("  " STR__(es) ":\n");
      PRINT_EXCEPTION64(xxx_mcontext);
    }
    if (xxx_mcontext64_size >=
        sizeof(xxx_mcontext64.__(es)) + sizeof(xxx_mcontext64.__(ss))) {
      printf("  " STR__(ss) ":\n");
      #define CPU_REG(rtype,name,ref) \
        printf("    " STR__(name) " = 0x%0*llX\n", \
               (int) sizeof(rtype) * 2, \
               (unsigned long long) xxx_mcontext64.__(ss).__(ref));
      CPU_REGISTERS64
      #undef CPU_REG
    }
    if (xxx_mcontext64_size >
        sizeof(xxx_mcontext64.__(es)) + sizeof(xxx_mcontext64.__(ss))) {
      printf("  [...]\n");
    }
    printf("\n");
  }
#endif
}

#else  /* !TESTED_CPU */

static void print_contexts(void) {}

#endif  /* !TESTED_CPU */

/* Determine whether we need to avoid the signal test */
static int
signals_broken(void)
{
#if BROKEN_SIGNALS
  return dlsym(RTLD_NEXT, "__MPLS_HAVE_PPC64_SIGNAL_FIX") == 0;
#else
  return 0;
#endif
}

int
main(int argc, char *argv[])
{
  int verbose = 0, err = 0;
  char *progname = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) print_macros();

  copy_contexts();
  copy_regs();
  if (!signals_broken()) {
    do {
      if ((err = get_sigcontext())) break;
      if (verbose) print_contexts();
      err = check_contexts();
    } while(0);
  } else {
    printf("  *** Skipping signal test in broken OS case.\n");
  }

  printf("%s %s on arch " EXPAND2(CPU_TYPE) ".\n",
         progname, err ? "failed" : "succeeded");
  return 0;
}
