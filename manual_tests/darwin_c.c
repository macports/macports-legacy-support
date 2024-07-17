/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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
 * This provides tests for the proper effects of __DARWIN_C_LEVEL.
 * Except for the initial macro definition tests, these involve providing
 * definitions which would conflict with the standard headers if the
 * normal declarations/definitions weren't suppressed by __DARWIN_C_LEVEL.
 * Since only one value of __DARWIN_C_LEVEL can be effective in a given build,
 * this entire test is wrapped multiple times with different values.
 * Since there's no easy way to defer such conflicts until runtime, all
 * failures (including the few that could be deferred) manifest themselves
 * as build failures.  The runtime test is just a dummy (almost).
 */

/* Make sure we have the SDK version for reporting. */
#include <_macports_extras/sdkversion.h>

/* First verify that the basic macros are defined. */

#include <sys/cdefs.h>

#ifndef __DARWIN_C_ANSI
#error __DARWIN_C_ANSI is not defined
#endif

#ifndef __DARWIN_C_FULL
#error __DARWIN_C_FULL is not defined
#endif

#ifndef __DARWIN_C_LEVEL
#error __DARWIN_C_LEVEL is not defined
#endif

/* Now test various conflict cases. */

/* dirent.h */
#include <dirent.h>
#if __DARWIN_C_LEVEL < 200809L
int fdopendir = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* stdio.h */
#include <stdio.h>
#if __DARWIN_C_LEVEL < 200809L
int dprintf = 0;
int getdelim = 0;
int getline = 0;
int open_memstream = 0;
int fmemopen = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* string.h */
#include <string.h>
#if __DARWIN_C_LEVEL < 200809L
int stpncpy = 0;
int strnlen = 0;
int strndup = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */
#if __DARWIN_C_LEVEL < __DARWIN_C_FULL
int memmem = 0;
#endif /* __DARWIN_C_LEVEL < __DARWIN_C_FULL */

/* strings.h */
#include <strings.h>
#if __DARWIN_C_LEVEL < __DARWIN_C_FULL
int ffsl = 0;
int ffsll = 0;
int fls = 0;
int flsl = 0;
int flsll = 0;
#endif /* __DARWIN_C_LEVEL < __DARWIN_C_FULL */

/* sys/fcntl.h */
#include <sys/fcntl.h>
#if __DARWIN_C_LEVEL < 200809L
#ifdef AT_FDCWD
#error AT_FDCWD is unexpectedly defined
#endif
#ifdef AT_FDCWD
#error AT_FDCWD is unexpectedly defined
#endif
#ifdef AT_EACCESS
#error AT_EACCESS is unexpectedly defined
#endif
#ifdef AT_SYMLINK_NOFOLLOW
#error AT_SYMLINK_NOFOLLOW is unexpectedly defined
#endif
#ifdef AT_SYMLINK_FOLLOW
#error AT_SYMLINK_FOLLOW is unexpectedly defined
#endif
#ifdef AT_REMOVEDIR
#error AT_REMOVEDIR is unexpectedly defined
#endif
int openat = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* sys/stat.h */
#include <sys/stat.h>
#if __DARWIN_C_LEVEL < 200809L
#ifdef UTIME_NOW
#error UTIME_NOW is unexpectedly defined
#endif
#ifdef UTIME_OMIT
#error UTIME_OMIT is unexpectedly defined
#endif
int futimens = 0;
int fstatat = 0;
int fstatat64 = 0;
int mkdirat = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* sys/stdio.h */
#include <sys/stdio.h>
#if __DARWIN_C_LEVEL < 200809L
int renameat = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/*
 * Apple inappropriately placed getattrlistat() in the 200809 category
 * in the 10.10-10.12 SDKs, though it's actually a Darwin extension.
 * We test it in the correct category where possible, but in the incorrect
 * category with 10.10-10.12 SDKs to avoid an Apple-induced failure.
 */
#define GETATTRLISTAT_APPLE_ERR (__MPLS_SDK_MAJOR >= 101000 \
                                 && __MPLS_SDK_MAJOR < 101300)
#if GETATTRLISTAT_APPLE_ERR  /* Dummy ref to suppress warning */
#endif

/* sys/unistd.h */
#include <sys/unistd.h>
#if __DARWIN_C_LEVEL < 200809L
int readlinkat = 0;
int faccessat = 0;
int fchownat = 0;
int linkat = 0;
int symlinkat = 0;
int unlinkat = 0;
#if GETATTRLISTAT_APPLE_ERR
int getattrlistat = 0;
#endif
#endif /* __DARWIN_C_LEVEL < 200809L */
#if __DARWIN_C_LEVEL < __DARWIN_C_FULL
#if !GETATTRLISTAT_APPLE_ERR
int getattrlistat = 0;
#endif
int setattrlistat = 0;
#endif /* __DARWIN_C_LEVEL < __DARWIN_C_FULL */

/* time.h */
#include <time.h>
#if __DARWIN_C_LEVEL < 199309L
#ifdef CLOCK_REALTIME
#error CLOCK_REALTIME is unexpectedly defined
#endif
#ifdef CLOCK_MONOTONIC
#error CLOCK_MONOTONIC is unexpectedly defined
#endif
#ifdef CLOCK_MONOTONIC_RAW
#error CLOCK_MONOTONIC_RAW is unexpectedly defined
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
#error CLOCK_MONOTONIC_RAW_APPROX is unexpectedly defined
#endif
#ifdef CLOCK_UPTIME_RAW
#error CLOCK_UPTIME_RAW is unexpectedly defined
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
#error CLOCK_UPTIME_RAW_APPROX is unexpectedly defined
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
#error CLOCK_PROCESS_CPUTIME_ID is unexpectedly defined
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
#error CLOCK_THREAD_CPUTIME_ID is unexpectedly defined
#endif
typedef void *clockid_t;
int clock_gettime = 0;
int clock_getres = 0;
#endif /* __DARWIN_C_LEVEL < 199309L */
#if !((__DARWIN_C_LEVEL >= __DARWIN_C_FULL) || \
          (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || \
          (defined(__cplusplus) && __cplusplus >= 201703L))
#ifdef TIME_UTC
#error TIME_UTC is unexpectedly defined
#endif
int timespec_get = 0;
#endif /* !__DARWIN_C_LEVEL ... */

/* unistd.h */
#include <unistd.h>
#if __DARWIN_C_LEVEL < __DARWIN_C_FULL
#ifdef _SC_NPROCESSORS_CONF
#error _SC_NPROCESSORS_CONF is unexpectedly defined
#endif
#ifdef _SC_NPROCESSORS_ONLN
#error _SC_NPROCESSORS_ONLN is unexpectedly defined
#endif
#ifdef _SC_PHYS_PAGES
#error _SC_PHYS_PAGES is unexpectedly defined
#endif
int fgetattrlist = 0;
int fsetattrlist = 0;
#endif /* __DARWIN_C_LEVEL < __DARWIN_C_FULL */

/* wchar.h */
#include <wchar.h>
#if __DARWIN_C_LEVEL < 200809L
int wcsdup = 0;
int wcsnlen = 0;
int wcpcpy = 0;
int wcpncpy = 0;
int wcscasecmp = 0;
int wcsncasecmp = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* xlocale/_wchar.h via wchar.h */
#if __DARWIN_C_LEVEL < 200809L
int wcscasecmp_l = 0;
int wcsncasecmp_l = 0;
#endif /* __DARWIN_C_LEVEL < 200809L */

/* Quasi-dummy runtime test, just reports values */

#include <stdio.h>

/* Get status of representative 11.x+ version macro. */
/* Not reentrant or thread-safe. */
static const char *
get_vermac(void)
{
  static char buf[128]; (void) buf;

  if (__MPLS_SDK_MAJOR < 110000) return "";
#ifndef MAC_OS_VERSION_11_0
  return ", MAC_OS_VERSION_11_0 is undefined";
#else
  sprintf(buf, ", MAC_OS_VERSION_11_0 = %d", MAC_OS_VERSION_11_0);
  return buf;
#endif
}

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("__DARWIN_C_LEVEL = %ld, __MPLS_SDK_MAJOR = %d%s\n",
         __DARWIN_C_LEVEL, __MPLS_SDK_MAJOR, get_vermac());
  return 0;
}
