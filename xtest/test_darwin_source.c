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
 * This provides tests for the proper effects of __DARWIN_C_SOURCE, by
 * testing for the expected [un]definitions based on the settings of
 * _ANSI_SOURCE, KERNEL, _POSIX_C_SOURCE, _XOPEN_SOURCE, and _DARWIN_C_SOURCE.
 * Since only one value of each flag can be effective in a given build,
 * this entire test is wrapped multiple times with different definitions.
 * Since there's no easy way to defer any conflicts until runtime, all
 * failures (including the few that could be deferred) manifest themselves
 * as build failures.  The runtime test is just a dummy (almost).
 */

/* Get target OS version */
#include <_macports_extras/targetos.h>

/* Make sure we have the SDK version for reporting. */
#include <_macports_extras/sdkversion.h>

/* Make sure we get any indirect definitions of the flags. */
#include <sys/cdefs.h>
/*
 * The $UNIX2003 versions of various calls are unavailable in 10.4, and
 * are supposed to be avoided when targeting 10.4.  Unfortunately, the
 * latter aspect doesn't work with some config-flag settings (which this
 * test needs), leading to link errors on 10.4 32-bit.  To avoid that, we
 * forcibly disable the suffix on 10.4.
 */
#if __MPLS_TARGET_OSVER < 1050 && defined(__DARWIN_SUF_UNIX03)
#undef __DARWIN_SUF_UNIX03
#define __DARWIN_SUF_UNIX03
#endif

/* Get this early for macro checks */
#include <dirent.h>

/* _ANSI_SOURCE alone cases */

#undef CHECK_FUNC

/* Combinations where definitions are expected */
#if !defined(_ANSI_SOURCE)

/* Verify that the function exists */

#define CHECK_FUNC(func) \
  __typeof__(func) *func##_ptr = func;

#else  /* Definitions not expected */

/* Get a conflict if the function exists */
#define CHECK_FUNC(func) \
  int func = 0;

#endif  /* Definitions not expected */

/***** Add tests here. *****/

/* _POSIX_C_SOURCE alone cases */

#undef CHECK_FUNC

/* Combinations where definitions are expected */
#if !defined(_POSIX_C_SOURCE) \
    || defined(_DARWIN_C_SOURCE)

/* Verify that the function exists */

#define CHECK_FUNC(func) \
  __typeof__(func) *func##_ptr = func;

/* Also check some macros */
#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#if !defined(DIRBLKSIZ) || !defined(DTF_HIDEW) \
    || !defined(DTF_NODUP) || !defined(DTF_REWIND) || !defined(__DTF_READALL)
#error missing dirent.h macros
#endif
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */

#else  /* Definitions not expected */

/* Get a conflict if the function exists */
#define CHECK_FUNC(func) \
  int func = 0;

/* Also check some macros */
#if defined(DIRBLKSIZ) || defined(DTF_HIDEW) \
    || defined(DTF_NODUP) || defined(DTF_REWIND) || defined(__DTF_READALL)
#error unexpected dirent.h macros
#endif

#endif  /* Definitions not expected */

/***** Add tests here. *****/

/* _ANSI_SOURCE + _POSIX_C_SOURCE cases */

#undef CHECK_FUNC

/* Combinations where definitions are expected */
#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) \
    || defined(_DARWIN_C_SOURCE))

/* Verify that the function exists */

#define CHECK_FUNC(func) \
  __typeof__(func) *func##_ptr = func;

#else  /* Definitions not expected */

/***** Add tests here. *****/

/* Get a conflict if the function exists */
#define CHECK_FUNC(func) \
  int func = 0;

#endif  /* Definitions not expected */

/* time.h */
#include <time.h>
CHECK_FUNC(posix2time)
#if !__DARWIN_UNIX03
CHECK_FUNC(timezone);
#endif /* !__DARWIN_UNIX03 */
CHECK_FUNC(tzsetwall)
CHECK_FUNC(time2posix)
CHECK_FUNC(timelocal)
CHECK_FUNC(timegm)

/***** Add more tests here. *****/

/* KERNEL + _POSIX_C_SOURCE cases */

#undef CHECK_FUNC

/* Combinations where definitions are expected */
#if !defined(KERNEL) \
    && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

/* Verify that the function exists */

#define CHECK_FUNC(func) \
  __typeof__(func) *func##_ptr = func;

#else  /* Definitions not expected */

/* Get a conflict if the function exists */
#define CHECK_FUNC(func) \
  int func = 0;

#endif  /* Definitions not expected */

/* dirent.h */

/* Conditions for these changed as of 10.8 */
#if __MPLS_SDK_MAJOR < 1080
CHECK_FUNC(alphasort)
CHECK_FUNC(__opendir2)
CHECK_FUNC(scandir)
#endif  /* __MPLS_SDK_MAJOR < 1080 */

/***** Add more tests here. *****/

/* _POSIX_C_SOURCE + _XOPEN_SOURCE cases */

#undef CHECK_FUNC

/* Combinations where definitions are expected */
#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) \
    || defined(_DARWIN_C_SOURCE)

/* Verify that the function exists */

#define CHECK_FUNC(func) \
  __typeof__(func) *func##_ptr = func;

#else  /* Definitions not expected */

/* Get a conflict if the function exists */
#define CHECK_FUNC(func) \
  int func = 0;

#endif  /* Definitions not expected */

/* pthread.h */
#include <pthread.h>
CHECK_FUNC(pthread_is_threaded_np)
CHECK_FUNC(pthread_main_np)
CHECK_FUNC(pthread_mach_thread_np)
CHECK_FUNC(pthread_get_stacksize_np)
CHECK_FUNC(pthread_get_stackaddr_np)
CHECK_FUNC(pthread_cond_signal_thread_np)
CHECK_FUNC(pthread_cond_timedwait_relative_np)
CHECK_FUNC(pthread_create_suspended_np)
CHECK_FUNC(pthread_kill)
CHECK_FUNC(pthread_sigmask)
CHECK_FUNC(pthread_yield_np)

/***** Add more tests here. *****/

/* Quasi-dummy runtime test, just reports values */

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;

#ifndef _ANSI_SOURCE
  printf("_ANSI_SOURCE is undef, ");
#else
  printf("_ANSI_SOURCE = %d, ", _ANSI_SOURCE);
#endif
#ifndef KERNEL
  printf("KERNEL is undef, ");
#else
  printf("KERNEL = %d, ", KERNEL);
#endif
#ifndef _POSIX_C_SOURCE
  printf("_POSIX_C_SOURCE is undef, ");
#else
  printf("_POSIX_C_SOURCE = %ld, ", _POSIX_C_SOURCE);
#endif
#ifndef _XOPEN_SOURCE
  printf("_XOPEN_SOURCE is undef\n");
#else
  printf("_XOPEN_SOURCE = %d\n", _XOPEN_SOURCE);
#endif
#ifndef _DARWIN_C_SOURCE
  printf("  _DARWIN_C_SOURCE is undef, ");
#else
  printf("  _DARWIN_C_SOURCE = %d, ", _DARWIN_C_SOURCE);
#endif
#ifndef __DARWIN_C_LEVEL
  printf("  __DARWIN_C_LEVEL is undef, ");
#else
  printf("  __DARWIN_C_LEVEL = %ld, ", __DARWIN_C_LEVEL);
#endif
  printf("__MPLS_TARGET_OSVER = %d, ", __MPLS_TARGET_OSVER);
  printf("__MPLS_SDK_MAJOR = %d\n", __MPLS_SDK_MAJOR);

  printf("%s succeeded.\n", basename(argv[0]));
  return 0;
}
