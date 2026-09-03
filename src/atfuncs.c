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

/* MP support header */
#include "MacportsLegacySupport.h"

#define __MPLS_LIB_SUPPORT_ALL_ATCALLS__ \
    (__MPLS_LIB_SUPPORT_ATCALLS__ \
     || __MPLS_LIB_SUPPORT_SETATTRLISTAT__ \
     || __MPLS_LIB_SUPPORT_UTIMENSAT__ \
     || __MPLS_LIB_SUPPORT_MKFIFONODAT__)

#if __MPLS_LIB_SUPPORT_ALL_ATCALLS__

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>

#include "atfuncs.h"

#if __MPLS_LIB_SUPPORT_ATCALLS__ || __MPLS_LIB_FAKE_PTHREAD_CHDIR__
/*
 * Get absolute path from dir fd and relative path into a supplied buffer
 * of size PATH_MAX.
 */
static int
get_abspath(int dirfd, const char *relative, char *buf)
{
  size_t len;

  if (dirfd == AT_FDCWD) {
    if (!getcwd(buf, PATH_MAX)) return -1;
  } else {
    if (fcntl(dirfd, F_GETPATH, buf)) return -1;
  }

  len = strlcat(buf, "/", PATH_MAX);
  if (len < PATH_MAX) len = strlcat(buf + len, relative, PATH_MAX - len);
  if (len >=PATH_MAX) {
    errno = ERANGE;
    return -1;
  }

  return 0;
}

#endif  /* __MPLS_LIB_SUPPORT_ATCALLS__ || __MPLS_LIB_FAKE_PTHREAD_CHDIR__ */

#if !__MPLS_LIB_FAKE_PTHREAD_CHDIR__

/*
 * ATFUNC wrapper functions (with pthread_fchdir)
 *
 * The ATFUNC wrapper is broken into two functions - one to set up the
 * directory for the underlying call, and one to restore the previous state.
 * These are normally invoked via the ATFUNC* macros, which may be used
 * elsewhere, so these functions are global.
 */

/*
 * Setup function:
 *
 * Checks whether dir manipulation is actually needed.
 * If so, opens "." to get an fd for the current dir.
 * Does a dir reset to see if a per-thread cwd was in effect.
 * Blocks all signals (while saving the previous state).
 * Switches to the target dir (with error handling).
 */

int
__mpls_atfunc_start(atfunc_t *at)
{
  int save_errno;
  sigset_t sigblock;

  if ((at->path && at->path[0] == '/') || at->dirfd == AT_FDCWD) {
    return 0;
  }

  at->orig = open(".", O_RDONLY);
  if (pthread_fchdir_np(-1) < 0 && at->orig >= 0) {
    (void) close(at->orig);
    at->orig = -1;
  }

  /* Block all signals for the duration */
  sigfillset(&sigblock);
  if (sigprocmask(SIG_BLOCK, &sigblock, &at->oldmask)) return -1;

  if (pthread_fchdir_np(at->dirfd) < 0) {
    save_errno = errno;
    if (at->orig >= 0) (void) close(at->orig);
    (void) sigprocmask(SIG_SETMASK, &at->oldmask, NULL);
    errno = save_errno;
    return -1;
  }

  at->restore = 1;
  return 0;
}

/*
 * Restore function:
 *
 * Returns immediately if nothing needs restoring.
 * Otherwise, restores the original cwd based on the saved fd (or -1).
 * Restores the original signal mask.
 * Closes the fd for the original cwd, if needed.
 */

void
__mpls_atfunc_finish(atfunc_t *at)
{
  int save_errno;

  if (!at->restore) return;

  save_errno = errno;
  (void) pthread_fchdir_np(at->orig);
  (void) sigprocmask(SIG_SETMASK, &at->oldmask, NULL);
  if (at->orig >= 0) (void) close(at->orig);
  errno = save_errno;
}

#else  /* __MPLS_LIB_FAKE_PTHREAD_CHDIR__ */

/*
 * ATFUNC wrapper functions (without pthread_fchdir)
 *
 * The ATFUNC wrapper is broken into two functions - one to create the
 * absolute path if needed, and a dummy one for restoring the previous state.
 * These are normally invoked via the ATFUNC* macros, which may be used
 * elsewhere, so these functions are global.
 */

/*
 * Setup function:
 *
 * Checks whether path adjustment is actually needed.
 * If so, obtains the directory path and constructs an absolute target path.
 * Sets the path for the wrapped function to be the absolute path.
 */

int
__mpls_atfunc_start(atfunc_t *at)
{
  int err;

  if ((at->path && at->path[0] == '/') || at->dirfd == AT_FDCWD) {
    return 0;
  }

  err = get_abspath(at->dirfd, at->path, at->abspath);
  if (err) return -1;

  at->path = at->abspath;
  return 0;
}

/*
 * Restore function:
 *
 * Does nothing since nothing needs to be restored.
 * This could still be a useful place for a breakpoint.
 */

void
__mpls_atfunc_finish(atfunc_t *at)
{
}

#endif  /* __MPLS_LIB_FAKE_PTHREAD_CHDIR__ */

#endif  /* __MPLS_LIB_SUPPORT_ALL_ATCALLS__ */

#if __MPLS_LIB_SUPPORT_ATCALLS__

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include <sys/stdio.h>  /* For renameat() */
#include <sys/stat.h>
#include <sys/unistd.h>

#include "util.h"

DEFINE_MPLS_ABORTMSG

int
faccessat(int dirfd, const char *pathname, int mode, int flags)
{
  int ret, save_errno;
  ATFUNC_VAR(dirfd, pathname);

  EINVAL_IF(flags & ~AT_EACCESS);

  uid_t ruid = getuid(), euid = geteuid();
  gid_t rgid = getgid(), egid = getegid();
  int check_euid = ruid != euid && (flags & AT_EACCESS);
  int check_egid = rgid != egid && (flags & AT_EACCESS);
  if (check_euid) setreuid(euid, ruid);
  if (check_egid) setregid(egid, rgid);

  ATFUNC_START(-1);
  ret = access(ATFUNC_PATH, mode);
  ATFUNC_FINISH;

  save_errno = errno;
  if (check_euid) setreuid(ruid, euid);
  if (check_egid) setregid(rgid, egid);
  errno = save_errno;

  return ret;
}

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  EINVAL_IF(flags & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (flags & AT_SYMLINK_NOFOLLOW) {
    ret = lchmod(ATFUNC_PATH, mode);
  } else {
    ret = chmod(ATFUNC_PATH, mode);
  }
  ATFUNC_FINISH;
  return ret;
}

int
fchownat(int dirfd, const char *pathname,
             uid_t owner, gid_t group, int flags)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  EINVAL_IF(flags & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (flags & AT_SYMLINK_NOFOLLOW) {
    ret = lchown(ATFUNC_PATH, owner, group);
  } else {
    ret = chown(ATFUNC_PATH, owner, group);
  }
  ATFUNC_FINISH;
  return ret;
}

/* For silly arch-dependent arg type */
#ifdef __LP64__
typedef unsigned int attrlist_opts_t;
#else /* !__LP64__ */
typedef unsigned long attrlist_opts_t;
#endif /* !__LP64__ */

int
getattrlistat(int dirfd, const char *pathname, void *a,
              void *buf, size_t size, unsigned long flags)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = getattrlist(ATFUNC_PATH, a, buf, size, (attrlist_opts_t) flags);
  ATFUNC_FINISH;
  return ret;
}

int
mkdirat(int dirfd, const char *pathname, mode_t mode)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = mkdir(ATFUNC_PATH, mode);
  ATFUNC_FINISH;
  return ret;
}

int
openat(int dirfd, const char *pathname, int flags, ...)
{
  int ret;
  mode_t mode;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  if (flags & O_CREAT) {
    va_list ap;
    va_start(ap, flags);
    mode = (mode_t)va_arg(ap, int);
    va_end(ap);
    ret = open(ATFUNC_PATH, flags, mode);
  } else {
    ret = open(ATFUNC_PATH, flags);
  }
  ATFUNC_FINISH;
  return ret;
}

/*
 * The $NOCANCEL version uses the corresponding open$NOCANCEL() if possible,
 * but falls back to the basic open() otherwise.  Having neither is fatal.
 */
int
openat$NOCANCEL(int dirfd, const char *pathname, int flags, ...)
{
  int ret;
  mode_t mode;
  ATFUNC_VAR(dirfd, pathname);
  GET_OS_OPT_ALT_FUNC(open, $NOCANCEL)

  ATFUNC_START(-1);
  if (flags & O_CREAT) {
    va_list ap;
    va_start(ap, flags);
    mode = (mode_t)va_arg(ap, int);
    va_end(ap);
    ret = (*os_open)(ATFUNC_PATH, flags, mode);
  } else {
    ret = (*os_open)(ATFUNC_PATH, flags);
  }
  ATFUNC_FINISH;
  return ret;
}

ssize_t
readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = readlink(ATFUNC_PATH, buf, bufsiz);
  ATFUNC_FINISH;
  return ret;
}

int
symlinkat(const char *oldpath, int newdirfd, const char *newpath)
{
  int ret;
  ATFUNC_VAR(newdirfd, newpath);

  ATFUNC_START(-1);
  ret = symlink(oldpath, ATFUNC_PATH);
  ATFUNC_FINISH;
  return ret;
}

int
unlinkat(int dirfd, const char *pathname, int flags)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  EINVAL_IF(flags & ~AT_REMOVEDIR);
  ATFUNC_START(-1);
  if (flags & AT_REMOVEDIR) {
    ret = rmdir(ATFUNC_PATH);
  } else {
    ret = unlink(ATFUNC_PATH);
  }
  ATFUNC_FINISH;
  return ret;
}

/*
 * The linkat() and renameat() functions are more complicated, since they
 * involve two directories and we can't set the cwd to two different values
 * simultaneously.  If either path is absolute, we can switch the cwd for
 * the other, and if both dirs are the same we can switch to that one.
 * Otherwise, we need to convert one to an absolute path.  We prefer to
 * switch the source cwd if there's a choice.
 *
 * The "same directory" check is based on the fds being the same; it doesn't
 * bother checking for the highly unlikely case where the two different fds
 * point to the same directory.  It's disabled in the 10.4 case (except when
 * it's AT_FDCWD), where relative paths always need to be made absolute, since
 * there's no cwd switching.
 *
 * The AT_LINK_NOFOLLOW flag is not supported, since the corresponding
 * feature is unavailable with link() (i.e., there's no llink() function).
 * We simply supply a dummy llink() that always fails.
 */

/* Selector for function type */
typedef enum rlmode_n {
  rlmode_rename,
  rlmode_link,
  rlmode_llink,
} rlmode_t;

/* Dummy llink()  */
static int
llink(const char *oldpath, const char *newpath)
{
  (void) oldpath; (void) newpath;
  errno = ENOTSUP;
  return -1;
}

/* Caller for selected function type */
static __inline__ int
rlfunc(const char *oldpath, const char *newpath, rlmode_t rlmode)
{
  switch (rlmode) {
    case rlmode_rename: return rename(oldpath, newpath);
    case rlmode_link: return link(oldpath, newpath);
    case rlmode_llink: return llink(oldpath, newpath);
  }
  errno = EINVAL; return -1;
}

/* Common handler for renameat and linkat */
static int
rlcommon(int olddirfd, const char *oldpath,
              int newdirfd, const char *newpath, rlmode_t rlmode)
{
  int ret;
  ATFUNC_VAR(olddirfd, oldpath);
  char absnew[PATH_MAX];

  /* If oldpath is absolute, apply ATFUNC to the new */
  if (oldpath && oldpath[0] == '/') {
    ATFUNC_SETFD(newdirfd, newpath);
    ATFUNC_START(-1);
    ret = rlfunc(oldpath, ATFUNC_PATH, rlmode);
    ATFUNC_FINISH;
    return ret;
  }

  /* If newpath is absolute or both fds are the same, apply ATFUNC to the old */
  if ((newpath && newpath[0] == '/') || ATFUNC_SAMEDIR(olddirfd, newdirfd)) {
    ATFUNC_START(-1);
    ret = rlfunc(ATFUNC_PATH, newpath, rlmode);
    ATFUNC_FINISH;
    return ret;
  }

  /* Otherwise, make newpath absolute and apply ATFUNC to the old */
  ret = get_abspath(newdirfd, newpath, absnew);
  if (ret) return ret;
  ATFUNC_START(-1);
  ret = rlfunc(ATFUNC_PATH, absnew, rlmode);
  ATFUNC_FINISH;
  return ret;
}

int
linkat(int olddirfd, const char *oldpath,
       int newdirfd, const char *newpath, int flags)
{
  EINVAL_IF(flags & ~AT_SYMLINK_NOFOLLOW);
  return rlcommon(olddirfd, oldpath, newdirfd, newpath,
     flags & AT_SYMLINK_NOFOLLOW ? rlmode_llink : rlmode_link);
}

int
renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath)
{
  return rlcommon(olddirfd, oldpath, newdirfd, newpath, rlmode_rename);
}

#endif  /* __MPLS_LIB_SUPPORT_ATCALLS__ */

#if __MPLS_LIB_SUPPORT_SETATTRLISTAT__

#include <stdint.h>

#include <sys/attr.h>

#include "atfuncs.h"

int
setattrlistat(int dirfd, const char *pathname, void *a, void *buf,
              size_t size, uint32_t flags)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = setattrlist(ATFUNC_PATH, a, buf, size, flags);
  ATFUNC_FINISH;
  return ret;
}

#endif  /* __MPLS_LIB_SUPPORT_SETATTRLISTAT__ */

#if __MPLS_LIB_SUPPORT_MKFIFONODAT__

#include <sys/stat.h>

#include "atfuncs.h"

int
mkfifoat(int dirfd, const char *pathname, mode_t mode)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = mkfifo(ATFUNC_PATH, mode);
  ATFUNC_FINISH;
  return ret;
}

int
mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev)
{
  int ret;
  ATFUNC_VAR(dirfd, pathname);

  ATFUNC_START(-1);
  ret = mknod(ATFUNC_PATH, mode, dev);
  ATFUNC_FINISH;
  return ret;
}

#endif  /* __MPLS_LIB_SUPPORT_MKFIFONODAT__ */

#if 0  /* Available as of macOS 27 */
int
fchflagsat(int dirfd, const char *path, int flags, int at_flags)
{
  int ret;
  ATFUNC_VAR(dirfd, path);

  EINVAL_IF(flags & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (at_flags & AT_SYMLINK_NOFOLLOW) {
    ret = lchflags(ATFUNC_PATH, flags);
  } else {
    ret = chflags(ATFUNC_PATH, flags);
  }
  ATFUNC_FINISH;
  return ret;
}
#endif
