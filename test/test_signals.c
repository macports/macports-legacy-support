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

#include <sys/mman.h>

/* Use SIGALRM so gdb doesn't intercept it by default */
#define TEST_SIG SIGALRM

typedef struct sigerr_s {
  const char *text;
  int error;
} sigerr_t;

typedef struct sigdata_s {
  int done;
  int test_ret;
  pid_t child;
  sigerr_t sigerr;
  stack_t sigstk;
  uint8_t stack[SIGSTKSZ];
} sigdata_t;

static sigdata_t *sigdatap;

int nofork = 0;

static void
simple_handler(int sig)
{
  (void) sig;
  sigdatap->done = 1;
}

static void
siginfo_handler(int sig, siginfo_t *info, void *uap)
{
  (void) sig; (void) info; (void) uap;
  sigdatap->done = 1;
}

typedef enum sigtype_n {
  sigtype_signal,
  sigtype_bsd,
  sigtype_sigaction,
  sigtype_sigaction_alt,
  sigtype_siginfo,
  sigtype_siginfo_alt,
  sigtype_max,
} sigtype_t;

/* Test the specified type of signal handling */
static int
test_signal(sigtype_t sigtype, int verbose)
{
  int err = 0;
  pid_t pid = getpid();
  struct sigaction act, oact;
  sigerr_t *sigerr = &sigdatap->sigerr;

  act.sa_mask = 0;
  sigdatap->done = 0;
  /* Clear sigstack so we know what was used (if we used it) */
  memset(sigdatap->stack, 0, sizeof(sigdatap->stack));

  switch (sigtype) {

  case sigtype_signal:
    sigerr->text = "signal()";
    err = signal(TEST_SIG, simple_handler) == SIG_ERR;
    break;

  case sigtype_bsd:
    sigerr->text = "bsd_signal()";
    err = bsd_signal(TEST_SIG, simple_handler) == SIG_ERR;
    break;

  case sigtype_sigaction:
    act.sa_handler = simple_handler;
    act.sa_flags = 0;
    sigerr->text = "sigaction() without SA_SIGINFO or SA_ONSTACK";
    err = sigaction(TEST_SIG, &act, &oact);
    break;

  case sigtype_sigaction_alt:
    act.sa_handler = simple_handler;
    act.sa_flags = SA_ONSTACK;
    sigerr->text = "sigaction() without SA_SIGINFO, with SA_ONSTACK";
    err = sigaction(TEST_SIG, &act, &oact);
    break;

  case sigtype_siginfo:
    act.sa_sigaction = siginfo_handler;
    act.sa_flags = SA_SIGINFO;
    sigerr->text = "sigaction() with SA_SIGINFO, no SA_ONSTACK";
    err = sigaction(TEST_SIG, &act, &oact);
    break;

  case sigtype_siginfo_alt:
    act.sa_sigaction = siginfo_handler;
    act.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigerr->text = "sigaction() with SA_SIGINFO and SA_ONSTACK";
    err = sigaction(TEST_SIG, &act, &oact);
    break;

  case sigtype_max:
    return 0;  /* Avoid possible unused case warning */
  }
  if (err) {
    sigerr->error = errno;
    return -1;
  }

  if (verbose) {
    printf("  Testing %s\n", sigerr->text);
    fflush(stdout);
  }

  err = kill(pid, TEST_SIG);
  if (err) {
    sigerr->text = "kill() for signal";
    sigerr->error = errno;
    (void) signal(TEST_SIG, SIG_DFL);
    return -1;
  }

  while (!sigdatap->done) ;

  if (signal(TEST_SIG, SIG_DFL) == SIG_ERR) {
    sigerr->text = "signal() to remove handler";
    sigerr->error = errno;
    return -1;
  }

  return 0;
}

/*
 * Since some failing cases may crash, by default we run the entire test
 * in a subprocess, so that crashes don't crash the entire program.  But
 * we provide an option to disable that, for less confusion when using
 * a debugger.
 */
static int
do_test_signal(sigtype_t sigtype, int verbose)
{
  pid_t child, done;
  int status;
  sigerr_t *sigerr;

  if (nofork) return test_signal(sigtype, verbose);

  sigerr = &sigdatap->sigerr;
  child = fork();
  if (child < 0) {
    sigerr->text = "fork()";
    sigerr->error = errno;
    return -1;
  }
  if (child == 0) {
    /* A debugger may introduce an intermediate process - note the real one */
    sigdatap->child = getpid();
    sigdatap->test_ret = test_signal(sigtype, verbose);
    exit(0);
  }
  do {
    /*
     * In the known failing case, we don't get the correct status, though
     * we do get nonzero status.
     */
    done = wait(&status);
    if (done != sigdatap->child) {
      /* There's some weird problem with debugging that this doesn't fix */
      if (done == -1) {
        if (errno == EINTR) continue;
        perror("    wait() failed");
      } else {
        /* With a debugger, done may != child */
        if (verbose) {
          fprintf(stderr, "    Unexpected wait() pid, %d != %d\n",
                  done, child);
        }
        continue;
      }
      exit(110);
    }
  } while (0);
  if (status) {
    sigerr->error = status;
    return -1;
  }
  return sigdatap->test_ret;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, err = 0;
  char *progname = basename(argv[0]);
  sigtype_t sigtype;
  sigerr_t *sigerr;
  stack_t *sigstk;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("Starting %s\n", progname);

  /* Put all relevant data in pages shared with subprocesses. */
  /* PROT_EXEC is unnecessary and may cause trouble in macOS 14+. */
  sigdatap = mmap(NULL, sizeof(sigdata_t),
                  PROT_READ | PROT_WRITE,
                  MAP_ANON | MAP_SHARED, -1, 0);
  if (sigdatap == MAP_FAILED) {
    perror("mmap() for data area failed");
    printf("%s failed.\n", progname);
    return 10;
  }
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
    err = do_test_signal(sigtype, verbose);
    if (err) {
      printf("    %s failed: %s\n", sigerr->text, strerror(sigerr->error));
      break;
    }
  }

  (void) munmap(sigdatap, sizeof(sigdata_t));

  printf("%s %s.\n", progname, err ? "failed" : "succeeded");
  return err != 0;
}
