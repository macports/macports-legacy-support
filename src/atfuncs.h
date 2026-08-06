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

#ifndef _MACPORTS_ATFUNCS_H_
#define _MACPORTS_ATFUNCS_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Macro to report EINVAL on a specified condition */
#define EINVAL_IF(cond) if (cond) { errno = EINVAL; return -1; }

/*
 * Two different methods are used to implement the "at" functions.  On
 * platforms with pthread_fchdir_np() (10.5+), we temporarily switch the
 * thread-local cwd to the target while performing the "non-at" version
 * of the function.
 *
 * In theory this could be used without the thread-local cwd if the
 * operations were suitably locked.  But this not only extends the locking
 * requirement to all functions that care about the cwd, but also seems to
 * run afoul of some undetermined bug that causes it to fail anyway.  So
 * instead we take the approach of obtaining the path of the target
 * directory and using it to construct the absolute path of the target.
 * This approach is already used in some cases of linkat() and renameat(),
 * since the cwd can't be switched to two different values simultaneously.
 */

#if !__MPLS_LIB_FAKE_PTHREAD_CHDIR__

#include <signal.h>

/* Struct containing data used by ATFUNC start/finish functions */
typedef struct atfunc_s {
  int dirfd;
  const char *path;
  int orig;
  int restore;
  sigset_t oldmask;
} atfunc_t;

/* Macro to set up ATFUNC data */
#define ATFUNC_VAR(fd, pth) \
  atfunc_t atv = { .dirfd = fd, .path = pth, .orig = -1, .restore = 0 };

/* Macro to check if both dirs are the same (linkat/renameat) */
#define ATFUNC_SAMEDIR(fd1, fd2) ((fd1) == (fd2))

#else  /* __MPLS_LIB_FAKE_PTHREAD_CHDIR__ */

#include <sys/syslimits.h>

/* Struct containing data used by ATFUNC start/finish functions */
typedef struct atfunc_s {
  int dirfd;
  const char *path;
  int orig;
  char abspath[PATH_MAX];
} atfunc_t;

/* Macro to set up ATFUNC data */
#define ATFUNC_VAR(fd, pth) \
  atfunc_t atv = { .dirfd = fd, .path = pth, .orig = -1 };

/* Macro to check if both dirs are the same and AT_FDCWD */
#define ATFUNC_SAMEDIR(fd1, fd2) ((fd1) == AT_FDCWD && (fd2) == AT_FDCWD)

#endif  /* __MPLS_LIB_FAKE_PTHREAD_CHDIR__ */

/* Macro for initial ATFUNC setup, with ret value for error) */
#define ATFUNC_START(errval) if (__mpls_atfunc_start(&atv)) return errval;

/* Macro to access path to be used */
#define ATFUNC_PATH (atv.path)

/* Macro to update fd and path in ATFUNC data */
#define ATFUNC_SETFD(fd, pth) atv.dirfd = fd; atv.path = pth;

/* Macro for ATFUNC cleanup */
#define ATFUNC_FINISH __mpls_atfunc_finish(&atv)

/* The actual functions */
extern int __mpls_atfunc_start(atfunc_t *at);
extern void __mpls_atfunc_finish(atfunc_t *at);

#endif /* _MACPORTS_ATFUNCS_H_ */
