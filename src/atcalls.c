/*
 * atcalls.c
 *
 * Copyright (c) 2013-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

 /*
  * NOTICE: This file was modified by Ken Cunningham in November 2018 to allow
  * for use as a supporting file for MacPorts legacy support library. This notice
  * is included in support of clause 2.2 (b) of the Apple Public License,
  * Version 2.0.
  */


/* MP support header */
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_ATCALLS__


#include <sys/attr.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ucred.h>
#include <sys/shm.h>
#include <sys/unistd.h>

#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


/* this is some apple internal magic */
#include <sys/syscall.h>
#include <fcntl.h>

#ifndef SYS___pthread_fchdir
# define SYS___pthread_fchdir 349
#endif

int __pthread_fchdir(int dirfd)
{
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
  return syscall(SYS___pthread_fchdir, dirfd);
#else
/* Tiger does not have kernel support for __pthread_fchdir, so we have to fall back to fchdir */
/* unless we can come up with a per-thread compatible implementation that works on Tiger */
  return syscall(SYS_fchdir, dirfd);
#endif
}


#define PROTECT_ERRNO(what)  ({ int __err = (errno); what; errno = __err; })
#define ERR_ON(code, what)   if (what) { errno = (code); return -1; }

#define _ATCALL(fd, p, onerr, what)                             \
    ({  typeof(what) __result;                                  \
        int oldCWD = -1;                                        \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            oldCWD = open(".", O_RDONLY);                       \
            if (__pthread_fchdir(-1) < 0 && oldCWD != -1) {     \
                close(oldCWD); oldCWD = -1;                     \
            }                                                   \
            if (__pthread_fchdir(fd) < 0) {                     \
                PROTECT_ERRNO(__pthread_fchdir(oldCWD));        \
                if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD)); \
                return onerr;                                   \
            }                                                   \
        }                                                       \
        __result = (what);                                      \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            PROTECT_ERRNO(__pthread_fchdir(oldCWD));            \
            if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));     \
        }                                                       \
        __result;                                               \
    })

#define ATCALL(fd, p, what)  _ATCALL(fd, p, -1, what)

// buf is a pointer to a buffer of PATH_MAX or larger size
static int
_fullpathat(int dirfd, const char *relative, char *buf)
{
    int cwd = -1;
    if (dirfd == AT_FDCWD) {
        cwd = open(".", O_RDONLY);
        if (cwd == -1)
            return -1;

        dirfd = cwd;
    }

    int ret;

    ret = fcntl(dirfd, F_GETPATH, buf);
    if (ret == -1)
        goto fail;

    strlcat(buf, "/", PATH_MAX);
    strlcat(buf, relative, PATH_MAX);

fail:
    if (cwd != -1)
        close(cwd);
    return ret;
}

int faccessat(int dirfd, const char *pathname, int mode, int flags)
{
    struct attrlist al = {
        .bitmapcount = ATTR_BIT_MAP_COUNT,
        .commonattr  = ATTR_CMN_USERACCESS,
    };
    struct {
        uint32_t length;
        uint32_t access;
    } __attribute__((aligned(4), packed)) ab;
    unsigned long opts = 0;

    ERR_ON(EINVAL, flags & ~AT_SYMLINK_NOFOLLOW);
    if (flags & AT_SYMLINK_NOFOLLOW)
        opts |= FSOPT_NOFOLLOW;
    if (getattrlistat(dirfd, pathname, &al, &ab, sizeof(ab), opts) < 0)
        return -1;
    ERR_ON(EACCES, (ab.access & mode) != (uint32_t)mode);
    return 0;
}

#if 0
int fchflagsat(int dirfd, const char *path, int flags, int at_flags)
{
    ERR_ON(EINVAL, at_flags & ~AT_SYMLINK_NOFOLLOW);
    if (at_flags & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(dirfd, path, lchflags(path, flags));
    } else {
        return ATCALL(dirfd, path, chflags(path, flags));
    }
}
#endif

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
{
    ERR_ON(EINVAL, flags & ~AT_SYMLINK_NOFOLLOW);
    if (flags & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(dirfd, pathname, lchmod(pathname, mode));
    } else {
        return ATCALL(dirfd, pathname, chmod(pathname, mode));
    }
}

int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags)
{
    ERR_ON(EINVAL, flags & ~AT_SYMLINK_NOFOLLOW);
    if (flags & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(dirfd, pathname, lchown(pathname, owner, group));
    } else {
        return ATCALL(dirfd, pathname, chown(pathname, owner, group));
    }
}

int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags)
{
    ERR_ON(EINVAL, flags & ~AT_SYMLINK_NOFOLLOW);
    if (flags & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(dirfd, pathname, lstat(pathname, buf));
    } else {
        return ATCALL(dirfd, pathname, stat(pathname, buf));
    }
}

/* 64bit inode types appeared only on 10.5, and currently can't be replaced on Tiger */
/* due to lack of kernel support for the underlying syscalls */

#if !__DARWIN_ONLY_64_BIT_INO_T && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
int fstatat64(int dirfd, const char *pathname, struct stat64 *buf, int flags)
{
    ERR_ON(EINVAL, flags & ~AT_SYMLINK_NOFOLLOW);
    if (flags & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(dirfd, pathname, lstat64(pathname, buf));
    } else {
        return ATCALL(dirfd, pathname, stat64(pathname, buf));
    }
}
#endif

int getattrlistat(int dirfd, const char *pathname, void *a,
                  void *buf, size_t size, unsigned long flags)
{
#ifdef __LP64__
    /* This is fricken stupid */
    unsigned int _flags = (unsigned int)flags;
    assert ((unsigned long)_flags == flags);

    return ATCALL(dirfd, pathname, getattrlist(pathname, a, buf, size, _flags));
#else
    return ATCALL(dirfd, pathname, getattrlist(pathname, a, buf, size, flags));
#endif
}

int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags)
{
    ERR_ON(EINVAL, flags);
    if (oldpath[0] == '/') {
        return ATCALL(newdirfd, newpath, link(oldpath, newpath));
    }
    if (newpath[0] == '/' || olddirfd == newdirfd) {
        return ATCALL(olddirfd, oldpath, link(oldpath, newpath));
    }

    // olddirfd != newdirfd and both relative
    int ret;

    char _oldpath[PATH_MAX];
    ret = _fullpathat(olddirfd, oldpath, _oldpath);
    if (ret == -1)
        return ret;

    char _newpath[PATH_MAX];
    ret = _fullpathat(newdirfd, newpath, _newpath);
    if (ret == -1)
        return ret;

    return link(_oldpath, _newpath);
}

int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
    return ATCALL(dirfd, pathname, mkdir(pathname, mode));
}

#if 0
int mkfifoat(int dirfd, const char *pathname, mode_t mode)
{
    return ATCALL(dirfd, pathname, mkfifo(pathname, mode));
}

int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev)
{
    return ATCALL(dirfd, pathname, mknod(pathname, mode, dev));
}
#endif

int openat(int dirfd, const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }

    return ATCALL(dirfd, pathname, open(pathname, flags, mode));
}

int openat$NOCANCEL(int dirfd, const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }

    return ATCALL(dirfd, pathname, open(pathname, flags, mode));
}

ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
    return ATCALL(dirfd, pathname, readlink(pathname, buf, bufsiz));
}

int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath)
{
    if (oldpath[0] == '/') {
        return ATCALL(newdirfd, newpath, rename(oldpath, newpath));
    }
    if (newpath[0] == '/' || olddirfd == newdirfd) {
        return ATCALL(olddirfd, oldpath, rename(oldpath, newpath));
    }

    // olddirfd != newdirfd and both relative
    int ret;

    char _oldpath[PATH_MAX];
    ret = _fullpathat(olddirfd, oldpath, _oldpath);
    if (ret == -1)
        return ret;

    char _newpath[PATH_MAX];
    ret = _fullpathat(newdirfd, newpath, _newpath);
    if (ret == -1)
        return ret;

    return rename(_oldpath, _newpath);
}

int symlinkat(const char *oldpath, int newdirfd, const char *newpath)
{
    return ATCALL(newdirfd, newpath, symlink(oldpath, newpath));
}

int unlinkat(int dirfd, const char *pathname, int flags)
{
    ERR_ON(EINVAL, flags & ~AT_REMOVEDIR);
    if (flags & AT_REMOVEDIR) {
        return ATCALL(dirfd, pathname, rmdir(pathname));
    } else {
        return ATCALL(dirfd, pathname, unlink(pathname));
    }
}

#endif  /* __MP_LEGACY_SUPPORT_ATCALLS__ */
