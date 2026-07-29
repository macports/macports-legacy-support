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
 * These tests exercise potential races in the "at" functions.  Since the
 * possible failures are probabilistic, they simply run for a specified
 * number of iterations or until a failure is detected.  Optionally,
 * there's a "no stop on error" mode, which runs for the full number of
 * iterations and reports the number of failures.  This is indicated by
 * specifying a negative iteration count.
 *
 * The first test checks the signal issue, by running an "at" function
 * rapidly while using ualarm() to generate a signal during the loop.
 * If the signal handler sees an incorrect cwd, that's an error.  Since
 * this test doesn't use threads, it's immune to thread-related issues.
 * Note that Rosetta does a poor job of handling signals, often causing
 * hangs during this test.
 *
 * The second test checks for thread collisions between "at" functions
 * by running two threads looping on fstatat() and checking the results.
 * This particular issue is solved by the lock used by the pthread_fchdir_np()
 * substitutes.
 *
 * The third test checks for thread collisions between "at" and "non-at"
 * functions, using fstatat() vs. getcwd().  This issue requires that the
 * "non-at" functions participate in the locking.
 *
 * None of these tests is exhaustive across all the relevant functions.
 * They simply test representative functions to determine whether the
 * framework handles the issues correctly.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

/* sysctl to check whether we're running natively (non-ppc only) */
#define SYSCTL_NATIVE "sysctl.proc_native"

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

/*
 * Delay for alarm in microseconds
 *
 * We want this delay to be long enough for the code to enter the fstatat()
 * loop before the signal occurs, but not unnecessarily long (to avoid slowing
 * the test down too much).  Observed mean delays from before the ualarm()
 * until after the first fstatat() range from ~1.5us to ~80us, so we use 100.
 * We could improve typical times by making the delay arch-specific, but since
 * this is only a manual test, it's not worth the trouble.
 */
#define SIG_DELAY 100

/* Use SIGALRM for testing; note gdb doesn't intercept it by default */
#define TEST_SIG SIGALRM

/* Communication area - shared to allow possible fork-based tests */
typedef struct info_s {
  int verbose;
  volatile int sigok;
  volatile int errcnt;
  volatile int errstat;
  int errline;
  volatile int numsigs;
  int origwdfd;
  int testwdfd;
  sig_t oldsig;
  struct stat origwdstat;
  struct stat testwdstat;
  struct stat teststat;
  char origwd[PATH_MAX];
  char testwd[PATH_MAX];
} info_t;

static info_t *info = NULL;

/* Status struct for thread */
typedef struct tstat_s {
  pthread_t thread;
  long count;
  int numiter;
  int errcnt;
  int errstat;
  int errline;
} tstat_t;

/* For debugging */
char *
get_cwd(void)
{
  static char dir[PATH_MAX];

  return getcwd(dir, PATH_MAX);
}

#if defined(__ppc__)

/* Test whether running under Rosetta */
/* -1 no, 1 yes */
static int
check_rosetta(void)
{
  int native;
  size_t native_sz = sizeof(native);
  static int rosetta = 0;

  if (rosetta) return rosetta;

  /* Check for Rosetta 1 */
  if (sysctlbyname(SYSCTL_NATIVE, &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    return (rosetta = -1);
  }
  return (rosetta = native ? -1 : 1);
}

#else  /* !ppc */

static int
check_rosetta(void)
{
  return -1;
}

#endif  /* !ppc */

static void
gotsig(int sig)
{
  (void) sig;
  info->sigok = 0;
  ++info->numsigs;
  if (!getcwd(info->testwd, PATH_MAX)) {
    info->errstat = errno;
    info->errline = __LINE__ - 2;
    ++info->errcnt;
  } else {
    if (strncmp(info->testwd, info->origwd, PATH_MAX)) ++info->errcnt;
  }
}

static int
check_signals(int count)
{
  long num = labs(count);
  sigset_t oldmask, newmask;

  if (!count) return 0;
  if (info->verbose) printf("  testing signals\n");
  if (check_rosetta() > 0) {
    printf("  !!! signal test may not work properly under Rosetta\n");
  }
  (void) fflush(NULL);

  info->sigok = 0;
  info->errcnt = 0;
  info->errstat = 0;
  info->numsigs = 0;

  if (sigprocmask(0, NULL, &oldmask)) {
    perror("  *** sigprocmask() failed");
    return 1;
  }

  info->oldsig = signal(TEST_SIG, gotsig);

  while (info->numsigs < num
         && !(info->errcnt && count >= 0) && !info->errstat) {
    info->sigok = 1;
    (void) ualarm(SIG_DELAY, 0);
    while (info->sigok) {
      (void) fstatat(info->testwdfd, ".", &info->teststat, 0);
    }
  }

  (void) signal(SIGSYS, info->oldsig);

  if (info->verbose) printf("    %d signals received\n", info->numsigs);
  if (info->errcnt) {
    if (info->errstat) {
      printf("  getcwd() within signal at line %d failed: %s\n",
             info->errline, strerror(info->errstat));
    }
    printf("  *** %d error%s detected\n",
           info->errcnt, info->errcnt == 1 ? "" : "s");
    return 1;
  }

  if (sigprocmask(0, NULL, &newmask)) {
    perror("  *** sigprocmask() failed");
    return 1;
  }
  if (newmask != oldmask) {
    printf("  *** signal mask clobbered");
    return 1;
  }

  return 0;
}

static void *
at_thread(void *tsarg)
{
  tstat_t *tstat = (tstat_t *) tsarg;
  long num = labs(tstat->count);
  struct stat sb;

  do {
    if (fstatat(info->testwdfd, ".", &sb, 0)) {
      tstat->errstat = errno;
      tstat->errline = __LINE__ - 2;
      ++tstat->errcnt;
    } else {
      if (sb.st_dev != info->testwdstat.st_dev
          || sb.st_ino != info->testwdstat.st_ino) {
        ++tstat->errcnt;
      }
    }
  } while (++tstat->numiter < num
           && !(tstat->errcnt && tstat->count >= 0) && !tstat->errstat);
  return NULL;
}

static void *
non_at_thread(void *tsarg)
{
  tstat_t *tstat = (tstat_t *) tsarg;
  long num = labs(tstat->count);
  char cwd[PATH_MAX];

  do {
    if (!getcwd(cwd, PATH_MAX)) {
      tstat->errstat = errno;
      tstat->errline = __LINE__ -2;
      ++tstat->errcnt;
    } else {
      if (strncmp(cwd, info->origwd, PATH_MAX)) {
        ++tstat->errcnt;
      }
    }
  } while (++tstat->numiter < num
           && !(tstat->errcnt && tstat->count >= 0) && !tstat->errstat);
  return NULL;
}

static int
check_at_threads(int count, int mode)
{
  int ret = 0, i;
  int numerrs = 0, totiters = 0;
  void *(*tfunc[2])(void *) = { at_thread, at_thread };
  tstat_t tstat[2] = {{0}};
  const char *ttype = mode ? "\"at\" vs. \"non-at\"" : "\"at\"";

  if (!count) return 0;
  if (info->verbose) {
    printf("  testing multithread %s function\n", ttype);
  }
  (void) fflush(NULL);

  /* Make sure we start with correct dir */
  if (fchdir(info->origwdfd)) {
    perror("  *** error resetting cwd\n");
    return 1;
  }

  if (mode) tfunc[1] = non_at_thread;

  for (i = 0; i < 2; ++i) {
    tstat[i].count = count;
    if (pthread_create(&tstat[i].thread, NULL, tfunc[i], &tstat[i])) {
      perror("  *** error creating thread");
      ret = 1;
      break;
    }
  }
  if (ret) {
    while (i--) (void) pthread_cancel(tstat[i].thread);
    return ret;
  }

  for (i = 0; i < 2; ++i) {
    if (pthread_join(tstat[i].thread, NULL)) {
      perror("  *** error joining thread");
      ret = 1;
    }
  }

  for (i = 0; i < 2; ++i) {
    numerrs += tstat[i].errcnt;
    totiters += tstat[i].numiter;
    if (tstat[i].errstat) {
      fprintf(stderr, "  *** thread %d at line %d got error %s\n",
              i, tstat[i].errline, strerror(tstat[i].errstat));
      ret = 1;
    }
  }
  if (!getcwd(info->testwd, PATH_MAX)) {
    perror("  *** getcwd() failed");
    ret = 1;
  }
  if (strncmp(info->testwd, info->origwd, PATH_MAX)) {
    printf("  *** %s test left cwd clobbered\n", ttype);
    ++numerrs;
  }
  if (info->verbose) printf("    %d combined iterations\n", totiters);
  if (numerrs) {
    printf("  *** %d %s error%s detected\n",
           numerrs, ttype, numerrs ==1 ? "" : "s");
    ret = 1;
  }

  return ret;
}

int
main(int argc, char *argv[])
{
  int ret = 0, verbose = 0, argn = 0, tnum = 0;
  char *cp;
  long counts[3] = {0};
  uint64_t start, end;
  double elapsed;
  char *progname = basename(argv[0]);

  while (++argn < argc && tnum < sizeof(counts) / sizeof(counts[0])) {
    if (!strcmp(argv[argn], "-v")) { ++verbose; continue; }
    counts[tnum++] = strtol(argv[argn], &cp, 0);
    if (*cp) {
      fprintf(stderr, "*** bad count '%s'\n", argv[argn]);
      printf("%s failed.\n", progname);
      return 20;
    }
  }

  if (verbose) printf("Starting %s\n", progname);

  start = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);

  do {
    /* Put all relevant data in pages shared with subprocesses. */
    /* PROT_EXEC is unnecessary and may cause trouble in macOS 14+. */
    info = mmap(NULL, sizeof(*info),
                  PROT_READ | PROT_WRITE,
                  MAP_ANON | MAP_SHARED, -1, 0);
    if (info == MAP_FAILED) {
      perror("  *** mmap() for info area failed");
      ret = 10;
      break;
    }
    info->verbose = verbose;
    info->origwdfd = info->testwdfd = -1;

    if (!getcwd(info->origwd, PATH_MAX)) {
      perror("  *** getcwd() failed");
      ret = 10;
      break;
    }
    info->origwdfd = open(".", O_RDONLY);
    if (info->origwdfd < 0) {
      perror("  *** open() for '.' failed");
      ret = 10;
      break;
    }
    if (fstat(info->origwdfd, &info->origwdstat)) {
      perror("  *** fstat() for '.' failed");
      ret = 10;
      break;
    }

    info->testwdfd = open(TEST_TEMP, O_RDONLY);
    if (info->testwdfd < 0) {
      perror("  *** open() for '" TEST_TEMP "' failed");
      ret = 10;
      break;
    }
    if (fstat(info->testwdfd, &info->testwdstat)) {
      perror("  *** fstat() for '" TEST_TEMP "' failed");
      ret = 10;
      break;
    }

    ret |= check_signals(counts[0]);
    ret |= check_at_threads(counts[1], 0);
    ret |= check_at_threads(counts[2], 1);

  } while(0);

  if (info) {
    if (info->testwdfd >= 0) (void) close(info->testwdfd);
    if (info->origwdfd >= 0) (void) close(info->origwdfd);
    (void) munmap(info, sizeof(*info));
  }

  end = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  elapsed = (end - start) / 1.0E9;

  printf("%s %s in %.3f seconds.\n",
         progname, ret ? "failed" : "passed", elapsed);
  return ret;
}
