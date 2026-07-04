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

#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ucontext.h>

#define SYSCTL_ALTIVEC "hw.optional.altivec"

#ifndef __ppc64__
#define OUR_UCONTEXT _STRUCT_UCONTEXT
#else
#define OUR_UCONTEXT _STRUCT_UCONTEXT64
#endif

#if defined(__ppc__) || defined(__ppc64__)
#define IS_PPCX 1
#else
#define IS_PPCX 0
#endif

/* Delay parameters (ms) */
#define DELAY_SETUP 20   /* Delay before setting up for signal */
#define DELAY_SIGNAL 20  /* Delay for signal in delay cases */

/* Use SIGVTALRM for setitimer(); also gdb doesn't intercept it by default */
#define TEST_SIG SIGVTALRM

/* Sample struct for typeof; global to avoid unused warning */
OUR_UCONTEXT uc_sample;

typedef struct sigerr_s {
  const char *text;
  int error;
} sigerr_t;

typedef struct sigdata_s {
  volatile int done;
  int test_ret;
  pid_t child;
  sigerr_t sigerr;
  __typeof__(uc_sample.uc_mcsize) mcsize;
  stack_t sigstk;
  uint8_t stack[SIGSTKSZ];
} sigdata_t;

static sigdata_t sigdata;

/* Reference the struct indirectly (allows for fork-based version) */
static sigdata_t *sigdatap = &sigdata;

static void
simple_handler(int sig)
{
  (void) sig;
  sigdatap->done = 1;
}

static void
siginfo_handler(int sig, siginfo_t *info, void *uap)
{
  (void) sig; (void) info;

  sigdatap->mcsize = ((OUR_UCONTEXT *) uap)->uc_mcsize;
  sigdatap->done = 1;
}

/*
 * Since our fix for the 10.4 ppc64 bug has separate cases for whether
 * or not vector context is included, we need to test both cases for
 * full test coverage.  Here we provide functions to activate and deactivate
 * the vector context switching (on ppcx machines).
 *
 * Merely setting vrsave nonzero isn't sufficient to enable vector context,
 * since setting vrsave isn't considered a "protected" AltiVec instrauction,
 * and hence doesn't trigger the exception needed to cause the kernel to
 * start tracking the vector context.  We need to execute at least one "true"
 * AltiVec instruction to do this, so we load v0 with a scratch value.
 *
 * For deactivation, we simply clear vrsave.  This doesn't seem to disable
 * AltiVec reliably (at least not immediately), so we put the vector cases
 * last.
 *
 * We follow changes to vrsave with an eieio, copying what the kernel does
 * in similar circumstances.  This may be unnecessary, but it doesn't hurt.
 *
 * We hand-assemble the AltiVec instructions to allow compiling without
 * AltiVec enabled in the compiler.
 */

#if IS_PPCX

static int
altivec_onoff(int want, int have)
{
  if (have) {
    if (want) {
      /* Enable saving, then fetch from the top of the stack to v0 */
      __asm__ __volatile__ (
          "\tlis r0, 65535\n"
          "\tori r0, r0, 65535\n"
          /* mtvrsave r0 */
          "\t.long 0x7c0043a6\n"
          "\teieio\n"
          /* lvx v0, 0, r1 */
          "\t.long 0x7c0008ce\n"
          ::
          );
    } else {
      /* Clear the vector save mask */
      __asm__ __volatile__ (
          "\tli r0, 0\n"
          /* mtvrsave r0 */
          "\t.long 0x7c0043a6\n"
          "\teieio\n"
          ::
          );
    }
    return 0;
  }
  return want;
}

#else

static int
altivec_onoff(int want, int have)
{
  (void) have;
  return want;
}

#endif

/* Test cases, for TEST_CASE(name,handler,flags,vec,delay,text) */
#define TEST_CASES \
  TEST_CASE(signal,simple,0,0,0,"signal()") \
  TEST_CASE(bsd,simple,0,0,0,"bsd_signal()") \
  TEST_CASE(sigaction,simple,0,0,0, \
       "sigaction() without SA_SIGINFO or SA_ONSTACK") \
  TEST_CASE(sigaction_alt,simple,SA_ONSTACK,0,0, \
       "sigaction() without SA_SIGINFO, with SA_ONSTACK") \
  TEST_CASE(siginfo,siginfo,SA_SIGINFO,0,0, \
       "sigaction() with SA_SIGINFO, no SA_ONSTACK") \
  TEST_CASE(siginfo_alt,siginfo,SA_SIGINFO|SA_ONSTACK,0,0, \
       "sigaction() with SA_SIGINFO and SA_ONSTACK") \
  TEST_CASE(siginfo_dly,siginfo,SA_SIGINFO,0,1, \
       "sigaction() with delayed SA_SIGINFO, no SA_ONSTACK") \
  TEST_CASE(siginfo_dly_alt,siginfo,SA_SIGINFO|SA_ONSTACK,0,1, \
       "sigaction() with delayed SA_SIGINFO and SA_ONSTACK") \
  TEST_CASE(siginfo_vec,siginfo,SA_SIGINFO,1,0, \
       "sigaction() with SA_SIGINFO and vec, no SA_ONSTACK") \
  TEST_CASE(siginfo_vec_alt,siginfo,SA_SIGINFO|SA_ONSTACK,1,0, \
       "sigaction() with SA_SIGINFO and vec and SA_ONSTACK") \
  TEST_CASE(siginfo_dly_vec,siginfo,SA_SIGINFO,1,1, \
       "sigaction() with delayed SA_SIGINFO and vec, no SA_ONSTACK") \
  TEST_CASE(siginfo_dly_vec_alt,siginfo,SA_SIGINFO|SA_ONSTACK,1,1, \
       "sigaction() with delayed SA_SIGINFO and vec and SA_ONSTACK") \

/* Test case enum */
#define TEST_CASE(name,handler,flags,vec,delay,text) sigtype_ ## name,
typedef enum sigtype_n {
  TEST_CASES
  sigtype_max,
} sigtype_t;
#undef TEST_CASE

/* Test case text */
#define TEST_CASE(name,handler,flags,vec,delay,text) text,
static const char * const test_text[] = {
  TEST_CASES
};
#undef TEST_CASE

/* Test case handlers */
#define TEST_CASE(name,handler,flags,vec,delay,text) &handler ## _handler,
static const void *test_handlers[] = {
  TEST_CASES
};
#undef TEST_CASE

/* Test case flags */
#define TEST_CASE(name,handler,flags,vec,delay,text) flags,
static const int test_flags[] = {
  TEST_CASES
};
#undef TEST_CASE

/* Test case vector wants */
#define TEST_CASE(name,handler,flags,vec,delay,text) vec,
static const int test_vecs[] = {
  TEST_CASES
};
#undef TEST_CASE

/* Test case delay wants */
#define TEST_CASE(name,handler,flags,vec,delay,text) delay,
static const int test_delays[] = {
  TEST_CASES
};
#undef TEST_CASE

/* Test the specified type of signal handling */
static int
test_signal(sigtype_t sigtype, int altivec, int verbose)
{
  int err = 0;
  pid_t pid = getpid();
  struct sigaction act, oact;
  sigerr_t *sigerr = &sigdatap->sigerr;
  struct itimerval itv;
  static struct timeval tv_zero = {0, 0};
  static struct timeval tv_delay = {0, DELAY_SIGNAL * 1000};

  sigerr->text = test_text[sigtype];
  sigdatap->done = 0;

  /* Clear sigstack so we know what was used (if we used it) */
  memset(sigdatap->stack, 0, sizeof(sigdatap->stack));

  act.sa_mask = 0;
  act.sa_handler = test_handlers[sigtype];
  act.sa_flags = test_flags[sigtype];

  /* Set up AltiVec or skip test */
  if (altivec_onoff(test_vecs[sigtype], altivec)) return 0;

  switch (sigtype) {

  case sigtype_signal:
    err = signal(TEST_SIG, simple_handler) == SIG_ERR;
    break;

  case sigtype_bsd:
    err = bsd_signal(TEST_SIG, simple_handler) == SIG_ERR;
    break;

  default:
    err = sigaction(TEST_SIG, &act, &oact);
  }

  if (err) {
    sigerr->error = errno;
    return -1;
  }

  if (verbose) {
    printf("  Testing %s\n", sigerr->text);
    fflush(stdout);
  }

  /* Get a fresh quantum so we don't prematurely reschedule */
  (void) usleep(DELAY_SETUP * 1000);

  if (!test_delays[sigtype]) {
    err = kill(pid, TEST_SIG);
    if (err) sigerr->text = "kill() for signal";
  } else {
    itv.it_interval = tv_zero;
    itv.it_value = tv_delay;
    err = setitimer(ITIMER_VIRTUAL, &itv, NULL);
    if (err) sigerr->text = "setitimer() for signal";
  }
  if (err) {
    sigerr->error = errno;
    (void) signal(TEST_SIG, SIG_DFL);
    return -1;
  }

  /* Wait for signal (if delayed); NOP if not delayed */
  while (!sigdatap->done) ;

  if (signal(TEST_SIG, SIG_DFL) == SIG_ERR) {
    sigerr->text = "signal() to remove handler";
    sigerr->error = errno;
    return -1;
  }

  if (verbose && act.sa_flags & SA_SIGINFO) {
    printf("    uc_mcsize = %zd = 0x%0*zX\n",
           sigdatap->mcsize,
           (int) sizeof(sigdatap->mcsize) * 2, sigdatap->mcsize);
  }

  return 0;
}

/* Check for AltiVec availability */

#if IS_PPCX

/* In the ppc* case, query the sysctl for AltiVec support */

static int
have_altivec(void)
{
  int val = 0;
  size_t vsiz = sizeof(val);

  if (sysctlbyname(SYSCTL_ALTIVEC, &val, &vsiz, NULL, 0) < 0) return 0;

  return val;
}

#else

/* In the non-ppc* case, AltiVec is obviously impossible. */

static int
have_altivec(void)
{
  return -1;
}

#endif

int
main(int argc, char *argv[])
{
  int verbose = 0, err = 0, altivec;
  char *progname = basename(argv[0]);
  sigtype_t sigtype;
  sigerr_t *sigerr;
  stack_t *sigstk;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("Starting %s\n", progname);

  altivec = have_altivec();

  sigerr = &sigdatap->sigerr;
  sigstk = &sigdatap->sigstk;
  sigstk->ss_sp = &sigdatap->stack;
  sigstk->ss_size = SIGSTKSZ;
  sigstk->ss_flags = 0;

  if (sigaltstack(sigstk, NULL)) {
    perror("sigaltstack() failed");
    printf("%s failed.\n", progname);
    return 10;
  }

  for (sigtype = 0; sigtype < sigtype_max; ++sigtype) {
    err = test_signal(sigtype, altivec, verbose);
    if (err) {
      printf("    %s failed: %s\n", sigerr->text, strerror(sigerr->error));
      break;
    }
  }

  if (verbose && altivec == 0) {
    printf("  AltiVec is unavailable - vector cases skipped\n");
  }

  printf("%s %s.\n", progname, err ? "failed" : "succeeded");
  return err != 0;
}
