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
 * This is a test of the syscall64() function.  Unlike most legacy-support
 * features, this is not a feature backported from a newer OS, but rather
 * a pure addition (albeit one that will help at least one backported
 * feature).
 *
 * Since no 64-bit-returning syscalls existed prior to 10.6 x86, we do
 * most of the tests with 32-bit syscalls, just to check the flow.  A test
 * with an actual 64-bit syscall is only a complete test on newer OSes.
 *
 * Note that Rosetta botches signal handling such that attempting to
 * catch the SIGSYS from an invalid syscall crashes the process.  So
 * when doing such tests under Rosetta, we wrap the test in a subprocess
 * to contain the crash.
 */

#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>

#include "_macports_extras/targetos.h"

/* Syscall for theadid - introduced in 10.6 */
#ifndef SYS_thread_selfid
#define SYS_thread_selfid 372
#endif

/* Minimum OS for threadid (never on ppc*) */
#if defined(__ppc__) || defined(__ppc64__)
#define MIN_THREADID_OS 9999999
#else
#define MIN_THREADID_OS 1060
#endif

/* Flag for big-endian 32-bit CPU */
#if defined(__ppc__)
#define BIG_ENDIAN_32 1
#else
#define BIG_ENDIAN_32 0
#endif

/* Type of pthread_threadid_np() (for old SDK) */
#define THREADID_FN  "pthread_threadid_np"
typedef int (pthread_threadid_fn)(pthread_t thread, uint64_t *thread_id);

/* sysctl to check whether we're running natively (non-ppc only) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* Use local name for syscall(), to avoid 10.12+ deprecation warnings */
/* Must be global to make compiler/linker happy */
int syscall32(int, ...) __asm("_syscall");

/* Structures for syscall results */

typedef struct scstat32_s {
  int sigsys;
  int errnum;
  int ret;
} scstat32_t;

typedef struct scstat64_s {
  int sigsys;
  int errnum;
  uint64_t ret;
} scstat64_t;

typedef struct scstatxx_s {
  scstat32_t ss32;
  scstat64_t ss64;
  int numsigs;
} scstatxx_t;

static scstatxx_t * volatile scstatp;

/* Signal handler (just sets flag) */

static volatile int intsys = 0;

static void
syssig(int signum)
{
  (void) signum;
  intsys = -1;
  ++scstatp->numsigs;
}

/* Type of signal handler pointer (signal() arg/result) */
typedef void (*sig_t)(int);

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

/* Syscall test functions */

typedef void (test_fn_t)(void);

/* Known successful syscall, using private functions */

static void
test_getpid32_priv(void)
{
  errno = 0;
  scstatp->ss32.ret = __mpls_syscall32(SYS_getpid);
  scstatp->ss32.errnum = errno;
}

static void
test_getpid64_priv(void)
{
  errno = 0;
  scstatp->ss64.ret = __mpls_syscall64(SYS_getpid);
  scstatp->ss64.errnum = errno;
}

/* Known successful syscall */

static void
test_getpid32(void)
{
  errno = 0;
  scstatp->ss32.ret = syscall32(SYS_getpid);
  scstatp->ss32.errnum = errno;
}

static void
test_getpid64(void)
{
  errno = 0;
  scstatp->ss64.ret = syscall64(SYS_getpid);
  scstatp->ss64.errnum = errno;
}

/* Known failing syscall (when given an intentionally bad arg) */

static void
test_chdir32(void)
{
  errno = 0;
  scstatp->ss32.ret = syscall32(SYS_chdir, NULL);
  scstatp->ss32.errnum = errno;
}

static void
test_chdir64(void)
{
  errno = 0;
  scstatp->ss64.ret = syscall64(SYS_chdir, NULL);
  scstatp->ss64.errnum = errno;
}

/* Known illegal syscall */

static void
test_illegal32(void)
{
  errno = 0;
  scstatp->ss32.ret = syscall32(-1);
  scstatp->ss32.errnum = errno;
}

static void
test_illegal64(void)
{
  errno = 0;
  scstatp->ss64.ret = syscall64(-1);
  scstatp->ss64.errnum = errno;
}

/* Test threadid syscall (if available) */
static void
test_threadid(void)
{
  errno = 0;
  scstatp->ss64.ret = syscall64(SYS_thread_selfid);
  scstatp->ss64.errnum = errno;
}

/* Test syscall, intercepting any failure directly */
static int
run_syscall_nf(test_fn_t *func)
{
  sig_t oldsig;

  oldsig = signal(SIGSYS, syssig);
  intsys = 0;
  (void) (*func)();
  (void) signal(SIGSYS, oldsig);
  return intsys;
}

/* Test syscall in a subprocess, to avoid Rosetta bug. */
static int
run_syscall_sp(test_fn_t *func)
{
  pid_t child, done;
  int status;
  sig_t oldsig;

  child = fork();
  if (child < 0) {
    perror("fork() failed");
    exit(100);
  }
  if (child == 0) {
    /* Try to catch signal, but it probably won't work */
    oldsig = signal(SIGSYS, syssig);
    intsys = 0;
    (void) (*func)();
    (void) signal(SIGSYS, oldsig);
    _exit(intsys);
  }
  do {
    done = wait(&status);
    if (done != child) {
      /* There's some weird problem with debugging that this doesn't fix */
      if (done == -1) {
        if (errno == EINTR) continue;
        perror("wait() failed");
      } else {
        /* With a debugger, done == child -1 */
        fprintf(stderr, "Unexpected wait() pid, %d != %d\n", done, child);
        break;
      }
      exit(110);
    }
  } while (0);
  return status;
}

/* Run one syscall function, optionally as a subprocess */
static int
run_syscall(test_fn_t *func, int usefork)
{
  return usefork ? run_syscall_sp(func) : run_syscall_nf(func);
}

/* Check 32- and 64-bit versions of a syscall against each other */
static int
check_syscall2(const char *name,
               test_fn_t *func32, test_fn_t*func64, int usefork, int verbose)
{
  int ret = 0, compare32;
  scstat32_t *sc32 = &scstatp->ss32;
  scstat64_t *sc64 = &scstatp->ss64;

  if (verbose) printf("  Testing %s\n", name);
  fflush(stdout);

  memset(scstatp, 0, sizeof(*scstatp));

  sc32->sigsys = run_syscall(func32, usefork);
  sc64->sigsys = run_syscall(func64, usefork);

  /* Get 32-bit version of 64-bit ret */
  #if BIG_ENDIAN_32
  compare32 = sc64->ret >> 32;
  #else
  compare32 = sc64->ret;
  #endif

  if (sc64->sigsys != sc32->sigsys) {
    printf("    sigsys mismatches, %d != %d\n", sc64->sigsys, sc32->sigsys);
    ret = 1;
  }
  if (sc64->errnum != sc32->errnum) {
    printf("    errno mismatches, %d != %d\n", sc64->errnum, sc32->errnum);
    ret = 1;
  }
  if (!sc32->sigsys && (int) compare32 != sc32->ret) {
    printf("    value mismatches, 0x%016llX != 0x%08X\n",
           (unsigned long long) sc64->ret, sc32->ret);
    ret = 1;
  }

  return ret;
}

/* Check one optional 64-bit-only syscall */
static int
check_syscall64(const char *name,
                test_fn_t *func, int min_os, int usefork, int verbose)
{
  scstat64_t *sc64 = &scstatp->ss64;

  if (verbose) printf("  Testing %s\n", name);
  fflush(stdout);

  memset(scstatp, 0, sizeof(*scstatp));
  sc64->sigsys = run_syscall(func, usefork);

  if (sc64->sigsys) {
    if (__MPLS_TARGET_OSVER >= min_os) {
      printf("    syscall is unexpectedly illegal\n");
      return -1;
    } else {
      if (verbose) printf("    syscall is not-unexpectedly illegal\n");
      return 1;
    }
  }

  /* If we forked to protect against SIGSYS, retry non-forked if legal */
  if (usefork) {
    memset(scstatp, 0, sizeof(*scstatp));
    sc64->sigsys = run_syscall(func, 0);

  }

  if (sc64->errnum || sc64->ret == ~0ULL) {
    printf("    returned an error, ret = %llu, errno = %d\n",
           (unsigned long long) sc64->ret, sc64->errnum);
    return -1;
  }

  return 0;
}

/* Check threadid syscall */
static int
check_threadid(int usefork, int verbose)
{
  int ret;
  pthread_threadid_fn *threadid_fn;
  uint64_t threadid;
  scstat64_t *sc64 = &scstatp->ss64;

  ret = check_syscall64("thread_selfid", test_threadid,
                        MIN_THREADID_OS, usefork, verbose);
  if (ret) return ret < 0;

  threadid_fn = dlsym(RTLD_NEXT, THREADID_FN);
  if (!threadid_fn) {
    if (verbose) {
      printf("    " THREADID_FN "() is unavailable\n");
      printf("      kernel threadid is %llu (0x%016llX)\n",
             (unsigned long long) sc64->ret,
             (unsigned long long) sc64->ret);
    }
    return 0;
  }

  ret = (*threadid_fn)(NULL, &threadid);
  if (ret) {
    perror("    " THREADID_FN "() failed");
    printf("      kernel threadid is %llu\n", (unsigned long long) threadid);
    return 1;
  }

  if (sc64->ret != threadid) {
    printf("    thread_selfid() mismatches " THREADID_FN "():"
           " %llu (0x%016llX) != %llu (0x%016llX)\n",
           (unsigned long long) sc64->ret, (unsigned long long) sc64->ret,
           (unsigned long long) threadid, (unsigned long long) threadid);
    return 1;
  } else {
    if (verbose) {
      printf("    threadid is %llu (0x%016llX)\n",
             (unsigned long long) threadid, (unsigned long long) threadid);
    }
  }

  return 0;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, usefork;
  char *progname = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("Starting %s\n", progname);

  /* Set up flag to use fork when we might get SIGSYS through Rosetta */
  usefork = check_rosetta() > 0;

  /* Put all relevant data in pages shared with subprocesses. */
  /* PROT_EXEC is unnecessary and may cause trouble in macOS 14+. */
  scstatp = mmap(NULL, sizeof(*scstatp),
                 PROT_READ | PROT_WRITE,
                 MAP_ANON | MAP_SHARED, -1, 0);
  if (scstatp == MAP_FAILED) {
    perror("mmap() for results area failed");
    printf("%s failed.\n", progname);
    return 10;
  }

  ret |= check_syscall2("32-bit getpid (with mpls funcs)",
                        test_getpid32_priv, test_getpid64_priv, 0, verbose);
  ret |= check_syscall2("32-bit getpid",
                        test_getpid32, test_getpid64, 0, verbose);
  ret |= check_syscall2("32-bit chdir",
                        test_chdir32, test_chdir64, 0, verbose);
  ret |= check_syscall2("ILLEGAL", test_illegal32, test_illegal64,
                        usefork, verbose);
  ret |= check_threadid(usefork, verbose);

  (void) munmap(scstatp, sizeof(*scstatp));

  printf("%s %s.\n", progname, ret ? "failed" : "succeeded");
  return ret;
}
