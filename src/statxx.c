/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

#if __MPLS_LIB_SUPPORT_STAT64__ || __MPLS_LIB_SUPPORT_ATCALLS__ \
   || __MPLS_LIB_FIX_TIGER_PPC64__

/* Common setup for all versions of *stat*() calls provided here */

/*
 * Cause our own refs to always use the 32-bit-inode variants.  This
 * wouldn't work on arm64, but is known to work on all platforms where
 * our implementations are needed.  It means that the referenced function
 * names are always the "unadorned" versions, except when we explicitly
 * add a suffix.
 *
 * This also makes the dlsym() interface common code.
 */

#define _DARWIN_NO_64_BIT_INODE 1

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/stat.h>

#include "compiler.h"

/* Make sure we have "struct stat64" */
#if !__MPLS_HAVE_STAT64
struct stat64 __DARWIN_STRUCT_STAT64;
#endif /* !__MPLS_HAVE_STAT64 */

#define GET_OS_FUNC(name) \
  static __typeof__(name) *os_##name = NULL; \
  \
  if (MPLS_SLOWPATH(!os_##name)) { \
    if (!(os_##name = dlsym(RTLD_NEXT, #name))) abort(); \
  }

#endif /* __MPLS_LIB_SUPPORT_... */

#if __MPLS_LIB_FIX_TIGER_PPC64__

/*
 * All *stat*() calls on 10.4 ppc64 usually return garbage in the tv_nsec
 * values of timestamps.  Since sub-second values are not actually supported
 * in this OS version, the fix is simply to add wrappers that clear all
 * tv_nsec values in the results.
 *
 * We arrange to pass through the result of the underlying *stat*() call,
 * for convenience.  We only apply the fix in the success case.
 */

/* Clear all tv_nsec values in the structure */
static __inline__ int
fix_stat(int result, struct stat *buf)
{
  if (MPLS_SLOWPATH(result)) return result;

  buf->st_atimespec.tv_nsec = 0;
  buf->st_mtimespec.tv_nsec = 0;
  buf->st_ctimespec.tv_nsec = 0;
  /* Note that the non-ino64 version has no birthtime. */

  return 0;
}

/* Now all the wrapper functions */

int
stat(const char *path, struct stat *buf)
{
  GET_OS_FUNC(stat)
  return fix_stat((*os_stat)(path, buf), buf);
}

int
lstat(const char *path, struct stat *buf)
{
  GET_OS_FUNC(lstat)
  return fix_stat((*os_lstat)(path, buf), buf);
}

int
fstat(int fildes, struct stat *buf)
{
  GET_OS_FUNC(fstat)
  return fix_stat((*os_fstat)(fildes, buf), buf);
}

/*
 * Since Rosetta (ppc) and ppc64 are mutually exclusive, we don't need
 * to worry about conflicts in the *statx_np() functions.
 */

int
statx_np(const char *path, struct stat *buf, filesec_t fsec)
{
  GET_OS_FUNC(statx_np)
  return fix_stat((*os_statx_np)(path, buf, fsec), buf);
}

int
lstatx_np(const char *path, struct stat *buf, filesec_t fsec)
{
  GET_OS_FUNC(lstatx_np)
  return fix_stat((*os_lstatx_np)(path, buf, fsec), buf);
}

int
fstatx_np(int fildes, struct stat *buf, filesec_t fsec)
{
  GET_OS_FUNC(fstatx_np)
  return fix_stat((*os_fstatx_np)(fildes, buf, fsec), buf);
}

#endif /* __MPLS_LIB_FIX_TIGER_PPC64__ */

#if __MPLS_LIB_FIX_TIGER_ROSETTA__
/*
 * The fstatx_np() function screws up when run under 10.4 Rosetta, due to
 * a missing byte swap of the fd.  This only happens with a non-NULL fsec
 * argument.  The fix is trivial, but determining when to apply it is not,
 * and is important, since applying the fix when it isn't needed would have
 * the effect of introducing the bug, rather than fixing it.
 *
 * The conditions where the bug is present are:
 *   1) Having a non-NULL fsec parameter.
 *   2) Running under Rosetta.
 *   3) This being the 10.4 Rosetta.
 *   4) The Rosetta bug's not having been fixed.
 *
 * Conditions 1 and 2 are easily established (#2 via an undocumented sysctl).
 * Condition 3 is tricky.  We can't just rely on the build-target OS
 * version, since it's legal to run code built for one OS version on a
 * later one.  Although we don't yet handle that issue in general, the
 * consequences of ignoring it would be bad, as noted, so we need to
 * actually check.
 * To make matters worse, not only is there no straightforward
 * way to obtain the Rosetta version, but even the libSystem version isn't
 * made available straightforwardly.  And condition 4 is a complete
 * wildcard, though it's highly unlikely that someone would have applied
 * a patch for such an esoteric bug on such an old system.
 *
 * The method we use here is:
 *   1) The check for the non-NULL fsec arg is trivial.
 *   2) We use a sysctl() to determine whether we're running under Rosetta.
 *   3) We use the presence of copyfile_init() to determine whether it's
 *  a 10.4 libSystem.  That function was part of the preliminary (unadvertised)
 *  copyfile implementation in 10.4, and was removed in 10.5.  It's highly
 *  unlikely that someone would have patched a later libSystem to add this
 *  obsolete function, so this is probably accurate.  In fact, we do provide
 *  this function in 10.5 as a compatibility aid, but that won't match
 *  dlsym(RTLD_NEXT,...), so it has no effect.
 *   4) We assume that the libSsystem version is a valid proxy for the
 *  Rosetta version.  That should be the case in any normal install, but
 *  if someone played mix-'n-match with system components, it might not be.
 *   5) We assume that the Rosetta bug has not been fixed.
 *
 * In principle, there's a more elaborate way to make the deteremination,
 * based on testing the actual behavior of the function, but for simplicity
 * we avoid that at the present, and use the conditions above.  If any of
 * the above assumptions were to become incorrect, we'd need to implement
 * the more elaborate scheme.  Of course, this entire section of code
 * is only present when building for 10.4 ppc.
 *
 * Note that we only need to provide the basic variant, since the wrappers
 * providing the additional variants will call this version.
 *
 * Also note that chmodx_np() is also broken in 10.4 Rosetta, but the nature of
 * the bug is unknown, and hence there's no fix provided.  It's known *not*
 * to be a simple byte-swapping issue.
 */

#include <stddef.h>

#include <sys/sysctl.h>
#include <sys/types.h>

#include <libkern/OSByteOrder.h>

/* sysctl to check whether we're running natively (non-ppc only) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* Function present in 10.4 and not later */
#define TIGER_TEST_FUNC "copyfile_init"

/* Test whether it's 10.4 Rosetta */
/* -1 no, 1 yes */
static int
is_tiger_rosetta(void)
{
  int native;
  size_t native_sz = sizeof(native);

  if (sysctlbyname(SYSCTL_NATIVE, &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    return -1;
  }
  if (native) return -1;
  /* Definitely Rosetta - see if it's 10.4. */
  return dlsym(RTLD_NEXT, TIGER_TEST_FUNC) ? 1 : -1;
}

/* Wrapper with optional byte swap of fd */
int
fstatx_np(int fildes, struct stat *buf, filesec_t fsec)
{
  static int tiger_rosetta = 0;
  GET_OS_FUNC(fstatx_np)

  /* If known not 10.4 Rosetta or NULL fsec, just call the function normally */
  if (MPLS_FASTPATH(tiger_rosetta < 0 || fsec == NULL)) {
    return (*os_fstatx_np)(fildes, buf, fsec);
  }

  /* If we don't yet know the Rosetta status, get it */
  if (!tiger_rosetta) tiger_rosetta = is_tiger_rosetta();

  /* Iff it's 10.4 Rosetta, byte-swap the fd */
  return (*os_fstatx_np)(tiger_rosetta >= 0 ? OSSwapInt32(fildes) : fildes,
                   buf, fsec);
}

#endif /* __MPLS_LIB_FIX_TIGER_ROSETTA__ */

#if __MPLS_LIB_SUPPORT_STAT64__

/*
 * This provides definitions for some 64-bit-inode function variants on 10.4.
 *
 * For the *stat() functions, this simply involves translating the result of
 * the 32-bit-inode variant.
 *
 * Providing a similar capability for directory-related functions would be
 * much more difficult, since the differently-formatted dirent struct is
 * provided via a pointer to an internal buffer, rather than one provided
 * by the caller.  Hence we don't do anything about those, for now.
 */

/* Do a field-by-field copy from ino32 stat to ino64 stat. */
/* Also provide passthrough for return value. */
static int
convert_stat(const struct stat *in, struct stat64 *out, int status)
{
  out->st_dev = in->st_dev;
  out->st_mode = in->st_mode;
  out->st_nlink = in->st_nlink;
  out->st_ino = in->st_ino;  /* Possible but unlikely overflow here */
  out->st_uid = in->st_uid;
  out->st_gid = in->st_gid;
  out->st_rdev = in->st_rdev;
  out->st_atimespec = in->st_atimespec;
  out->st_mtimespec = in->st_mtimespec;
  out->st_ctimespec = in->st_ctimespec;
  /* The ino32 stat doesn't have birthtime, so use MIN(ctime, mtime) */
  if (in->st_ctimespec.tv_sec < in->st_mtimespec.tv_sec
      || (in->st_ctimespec.tv_sec == in->st_mtimespec.tv_sec
          && in->st_ctimespec.tv_nsec < in->st_mtimespec.tv_nsec)) {
    out->st_birthtimespec = in->st_ctimespec;
  } else {
    out->st_birthtimespec = in->st_mtimespec;
  }
  out->st_size = in->st_size;
  out->st_blocks = in->st_blocks;
  out->st_blksize = in->st_blksize;
  out->st_flags = in->st_flags;
  out->st_gen = in->st_gen;
  out->st_lspare = 0;
  out->st_qspare[0] = 0;
  out->st_qspare[1] = 0;
  return status;
}

int
stat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, stat(path, &stbuf));
}

int
lstat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstat(path, &stbuf));
}

int
fstat$INODE64(int fildes, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstat(fildes, &stbuf));
}

int
statx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                 filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, statx_np(path, &stbuf, fsec));
}

int
lstatx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                  filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstatx_np(path, &stbuf, fsec));
}

int
fstatx_np$INODE64(int fildes, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstatx_np(fildes, &stbuf, fsec));
}

#if __MPLS_HAVE_STAT64

int
stat64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, stat(path, &stbuf));
}

int
lstat64(const char *__restrict path, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstat(path, &stbuf));
}

int
fstat64(int fildes, struct stat64 *buf)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstat(fildes, &stbuf));
}

int
statx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, statx_np(path, &stbuf, fsec));
}

int
lstatx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, lstatx_np(path, &stbuf, fsec));
}

int
fstatx64_np(int fildes, struct stat64 *buf, filesec_t fsec)
{
  struct stat stbuf;
  return convert_stat(&stbuf, buf, fstatx_np(fildes, &stbuf, fsec));
}

#endif /* __MPLS_HAVE_STAT64 */

#endif /* __MPLS_LIB_SUPPORT_STAT64__*/

#if __MPLS_LIB_SUPPORT_ATCALLS__

/*
 * Provide "at" versions of the *stat*() calls, on OS versions that don't
 * provide them natively.
 */

int stat$INODE64(const char *__restrict path, struct stat64 *buf);
int lstat$INODE64(const char *__restrict path, struct stat64 *buf);

#include "atcalls.h"

int fstatat(int fd, const char *__restrict path, struct stat *buf, int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat(path, buf));
    } else {
        return ATCALL(fd, path, stat(path, buf));
    }
}

int fstatat$INODE64(int fd, const char *__restrict path, struct stat64 *buf,
                    int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat$INODE64(path, buf));
    } else {
        return ATCALL(fd, path, stat$INODE64(path, buf));
    }
}

#if __MPLS_HAVE_STAT64

/*
 * The fstatat64 function is not expected to be accessed directly (though many
 * system libraries provide it as a convenience synonym for fstatat$INODE64),
 * so no SDK provides a prototype for it.  We do so here.
 */

extern int fstatat64(int fd, const char *__restrict path,
                     struct stat64 *buf, int flag);

int fstatat64(int fd, const char *path, struct stat64 *buf, int flag)
{
    ERR_ON(EINVAL, flag & ~AT_SYMLINK_NOFOLLOW);
    if (flag & AT_SYMLINK_NOFOLLOW) {
        return ATCALL(fd, path, lstat64(path, buf));
    } else {
        return ATCALL(fd, path, stat64(path, buf));
    }
}

#endif /* __MPLS_HAVE_STAT64 */

#endif  /* __MPLS_LIB_SUPPORT_ATCALLS__ */
