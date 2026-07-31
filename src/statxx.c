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
 */

#define _DARWIN_NO_64_BIT_INODE 1

#include <stddef.h>
#include <stdlib.h>

#include <sys/stat.h>

#include "util.h"

/* Make sure we have "struct stat64" */
#if !__MPLS_HAVE_STAT64
struct stat64 __DARWIN_STRUCT_STAT64;
#endif /* !__MPLS_HAVE_STAT64 */

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
 * We now make use of the centralized Rosetta tracking mechanism in util.c
 * to determine conditions 2-4.  Condition 1 obviously needs to be
 * determined here.
 *
 * Note that we only need to provide the basic variant, since the wrappers
 * providing the additional variants will call this version.
 */

#include <stddef.h>

#include <sys/sysctl.h>
#include <sys/types.h>

#include <libkern/OSByteOrder.h>

#include "rosetta.h"

#define NEED_FSTATX_SWAP   (__mpls_rosetta1_bugs & _ROSETTA1_BUG_FSTATX_FD)
#define NEED_FCHMODX_SWAP  (__mpls_rosetta1_bugs & _ROSETTA1_BUG_FCHMODX_FD)
#define SWAP_FD(x)         OSSwapInt32(x)

/* Wrapper for fstatx_np() with optional byte swap of fd */
int
fstatx_np(int fildes, struct stat *buf, filesec_t fsec)
{
  GET_OS_FUNC(fstatx_np)

  /* If known not the bug case or NULL fsec, just call the function normally */
  if (MPLS_FASTPATH(!NEED_FSTATX_SWAP || fsec == NULL)) {
    return (*os_fstatx_np)(fildes, buf, fsec);
  }
  /* Else call the function with the byte-swapped fd */
  return (*os_fstatx_np)(SWAP_FD(fildes), buf, fsec);
}

#else  /* !__MPLS_LIB_FIX_TIGER_ROSETTA__ */

#define NEED_FCHMODX_SWAP 0
#define SWAP_FD(x)        (x)

#endif  /* !__MPLS_LIB_FIX_TIGER_ROSETTA__ */

#if __MPLS_LIB_FIX_TIGER_CHMODX__
/*
 * The [f]chmodx_np() functions have issues on 10.4, though the exact nature
 * isn't fully understood.  What is known is:
 *   1) fchmodx_np() on 10.4 Rosetta has the same fd byte-swapping issue
 *  as fstatx_np(), but unconditionally.
 *   2) fchmodx_np() fails the simple chmodx test in all 10.4 cases, though
 *  chmodx_np() passes in non-Rosetta cases.
 *   3) chmodx_np() on 10.4 Rosetta also fails the simple test.
 *
 * It's unknown at this point whether the problems can be completely
 * corrected without a kernel fix.  The approach taken here is to try to
 * use the traditional [f]chmod()/[f]chown() functions whenever extended
 * properties don't need to be changed.  This is accomplished by reading
 * the current properties with [f]statx_np() and comparing them to the
 * desired new values.
 *
 * While this approach is inadequate for the general case, it means that
 * programs which use [f]chmod_np() just because they *may* need to change
 * extended properties can function correctly when that isn't the case.
 */

#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

#include "filesec_internal.h"

#define NEED_CHMODX 1
#define NEED_CHMOD  2
#define NEED_CHOWN  4

/* Determine whether [f]chmodx is actually needed */
static int
need_chmodx(struct _filesec *old, struct _filesec *new, struct stat *sb)
{
  int need = 0;

  /* See if uuid change wanted */
  if (new->fs_valid & FS_VALID_UUID) {
    if (!(old->fs_valid & FS_VALID_UUID)) return NEED_CHMODX;
    if (bcmp(&new->fs_uuid, &old->fs_uuid, sizeof(old->fs_uuid))) {
      return NEED_CHMODX;
    }
  }
  /* See if grpuuid change wanted */
  if (new->fs_valid & FS_VALID_GRPUUID) {
    if (!(old->fs_valid & FS_VALID_GRPUUID)) return NEED_CHMODX;
    if (bcmp(&new->fs_grpuuid, &old->fs_grpuuid, sizeof(old->fs_grpuuid))) {
      return NEED_CHMODX;
    }
  }
  /* See if ACL change wanted */
  if (new->fs_valid & FS_VALID_ACL) {
    if (!(old->fs_valid & FS_VALID_ACL)) return NEED_CHMODX;
    if (new->fs_aclsize != old->fs_aclsize) return NEED_CHMODX;
    /* Note that different ACL order will count as a mismatch */
    if (bcmp(new->fs_aclbuf, old->fs_aclbuf, old->fs_aclsize)) {
      return NEED_CHMODX;
    }
  }
  /* No extended changes - just update the traditional stat as needed */
  if (new->fs_valid & FS_VALID_UID && sb->st_uid != new->fs_uid) {
    sb->st_uid = new->fs_uid;
    need |= NEED_CHOWN;
  } else {
    sb->st_uid = -1;
  }
  if (new->fs_valid & FS_VALID_GID && sb->st_gid != new->fs_gid) {
    sb->st_gid = new->fs_gid;
    need |= NEED_CHOWN;
  } else {
    sb->st_gid = -1;
  }
  if (new->fs_valid & FS_VALID_MODE && sb->st_mode != new->fs_mode) {
    sb->st_mode = new->fs_mode;
    need |= NEED_CHMOD;
  }
  return need;
}

/* Wrapper for chmodx_np() with fixes */
int
chmodx_np(const char *path, filesec_t fsec)
{
  int need, err = 0;
  struct stat sb;
  struct _filesec *fsec_cur;
  GET_OS_FUNC(chmodx_np)

  /* If fsec is NULL, just call the OS function (probably illegal) */
  if (!fsec) return (*os_chmodx_np)(path, fsec);

  /* Get another filesec_t for reading */
  fsec_cur = filesec_init();
  if (!fsec_cur) return -1;

  /* Get the current status of the file */
  if (statx_np(path, &sb, fsec_cur)) return -1;

  /* See what changes are needed */
  need = need_chmodx(fsec_cur, fsec, &sb);
  filesec_free(fsec_cur);

  if (need & NEED_CHMODX) return (*os_chmodx_np)(path, fsec);

  if (need & NEED_CHMOD) err = chmod(path, sb.st_mode);
  if (err) return -1;

  if (need & NEED_CHOWN) err = chown(path, sb.st_uid, sb.st_gid);

  return err;
}

/* Wrapper for fchmodx_np() with fixes */
int
fchmodx_np(int fildes, filesec_t fsec)
{
  int fd_adj = NEED_FCHMODX_SWAP ? SWAP_FD(fildes) : fildes;
  int need, err = 0;
  struct stat sb;
  struct _filesec *fsec_cur;
  GET_OS_FUNC(fchmodx_np)

  /* If fsec is NULL, just call the OS function (probably illegal) */
  if (!fsec) return (*os_fchmodx_np)(fd_adj, fsec);

  /* Get another filesec_t for reading */
  fsec_cur = filesec_init();
  if (!fsec_cur) return -1;

  /* Get the current status of the file */
  if (fstatx_np(fildes, &sb, fsec_cur)) return -1;

  /* See what changes are needed */
  need = need_chmodx(fsec_cur, fsec, &sb);
  filesec_free(fsec_cur);

  if (need & NEED_CHMODX) return (*os_fchmodx_np)(fd_adj, fsec);

  if (need & NEED_CHMOD) err = fchmod(fildes, sb.st_mode);
  if (err) return -1;

  if (need & NEED_CHOWN) err = fchown(fildes, sb.st_uid, sb.st_gid);

  return err;
}

#endif  /* __MPLS_LIB_FIX_TIGER_CHMODX__ */

#if __MPLS_LIB_SUPPORT_STAT64__

/*
 * This provides definitions for some 64-bit-inode function variants on 10.4.
 *
 * Providing a similar capability for directory-related functions would be
 * much more difficult, since the differently-formatted dirent struct is
 * provided via a pointer to an internal buffer, rather than one provided
 * by the caller.  Hence we don't do anything about those, for now.
 *
 * For the *stat() functions, this simply involves translating the result of
 * the 32-bit-inode variant.
 *
 * Since the caller-supplied stat64 buffer is larger than the stat buffer
 * needed by the syscall, we can use it directly for the syscall, thereby
 * (mostly) leveraging the OS address validation.  But since the fields
 * are in a different order, we can't directly reformat it in place, and
 * instead need to make a temporary copy as an intermediary.  Although this
 * seems like extra overhead, it's far less expensive than performing the
 * buffer address validation in userspace.
 *
 * The "mostly" comes about because if a page boundary lands between the
 * end of the stat and the end of the stat64, we haven't fully validated the
 * buffer.  We call the validation function for extra portion, with the
 * known valid start address as a hint, which will usually avoid the full
 * validation sequence.
 *
 * For some unknown reason, optimized code was sometimes screwing up the
 * result until the extra clearing step was inserted, even though there's
 * no good reason why this should be necessary (there's no type punning
 * involved).
 */

#include <errno.h>

typedef union stat_buf_u {
  struct stat s;
  struct stat64 s64;
} stat_buf_t;

/* Do a field-by-field copy from ino32 stat to ino64 stat. */
/* Also provide passthrough for return value. */
static int
convert_stat(int result, stat_buf_t *sb)
{
  struct stat stbuf;
  static const struct stat64 s64zero = {0};

  if (MPLS_SLOWPATH(result)) return result;

  if (__mpls_check_access(&sb->s + 1, sizeof(sb->s64) - sizeof(sb->s),
      VM_PROT_WRITE, &sb->s)) {
    errno = EFAULT;
    return -1;
  }

  stbuf = sb->s;
  /* Start with all-zero result (avoid weird optimizer bug) */
  sb->s64 = s64zero;

  sb->s64.st_dev = stbuf.st_dev;
  sb->s64.st_mode = stbuf.st_mode;
  sb->s64.st_nlink = stbuf.st_nlink;
  sb->s64.st_ino = stbuf.st_ino;  /* Possible but unlikely overflow here */
  sb->s64.st_uid = stbuf.st_uid;
  sb->s64.st_gid = stbuf.st_gid;
  sb->s64.st_rdev = stbuf.st_rdev;
  sb->s64.st_atimespec = stbuf.st_atimespec;
  sb->s64.st_mtimespec = stbuf.st_mtimespec;
  sb->s64.st_ctimespec = stbuf.st_ctimespec;
  /* The ino32 stat doesn't have birthtime, so use MIN(ctime, mtime) */
  if (stbuf.st_ctimespec.tv_sec < stbuf.st_mtimespec.tv_sec
      || (stbuf.st_ctimespec.tv_sec == stbuf.st_mtimespec.tv_sec
          && stbuf.st_ctimespec.tv_nsec < stbuf.st_mtimespec.tv_nsec)) {
    sb->s64.st_birthtimespec = stbuf.st_ctimespec;
  } else {
    sb->s64.st_birthtimespec = stbuf.st_mtimespec;
  }
  sb->s64.st_size = stbuf.st_size;
  sb->s64.st_blocks = stbuf.st_blocks;
  sb->s64.st_blksize = stbuf.st_blksize;
  sb->s64.st_flags = stbuf.st_flags;
  sb->s64.st_gen = stbuf.st_gen;
  /* Copy the "do not use" spares verbatim as well */
  sb->s64.st_lspare = stbuf.st_lspare;
  sb->s64.st_qspare[0] = stbuf.st_qspare[0];
  sb->s64.st_qspare[1] = stbuf.st_qspare[1];

  return result;
}

int
stat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(stat(path, &sb->s), sb);
}

int
lstat$INODE64(const char *__restrict path, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(lstat(path, &sb->s), sb);
}

int
fstat$INODE64(int fildes, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(fstat(fildes, &sb->s), sb);
}

int
statx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                 filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(statx_np(path, &sb->s, fsec), sb);
}

int
lstatx_np$INODE64(const char *__restrict path, struct stat64 *buf,
                  filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(lstatx_np(path, &sb->s, fsec), sb);
}

int
fstatx_np$INODE64(int fildes, struct stat64 *buf, filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(fstatx_np(fildes, &sb->s, fsec), sb);
}

#if __MPLS_HAVE_STAT64

int
stat64(const char *__restrict path, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(stat(path, &sb->s), sb);
}

int
lstat64(const char *__restrict path, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(lstat(path, &sb->s), sb);
}

int
fstat64(int fildes, struct stat64 *buf)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(fstat(fildes, &sb->s), sb);
}

int
statx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(statx_np(path, &sb->s, fsec), sb);
}

int
lstatx64_np(const char *__restrict path, struct stat64 *buf, filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(lstatx_np(path, &sb->s, fsec), sb);
}

int
fstatx64_np(int fildes, struct stat64 *buf, filesec_t fsec)
{
  stat_buf_t *sb = (stat_buf_t *) buf;
  return convert_stat(fstatx_np(fildes, &sb->s, fsec), sb);
}

#endif /* __MPLS_HAVE_STAT64 */

#endif /* __MPLS_LIB_SUPPORT_STAT64__*/

#if __MPLS_LIB_SUPPORT_ATCALLS__

/*
 * Provide "at" versions of the *stat*() calls, on OS versions that don't
 * provide them natively.
 */

#include <errno.h>

#include <sys/fcntl.h>

int stat$INODE64(const char *__restrict path, struct stat64 *buf);
int lstat$INODE64(const char *__restrict path, struct stat64 *buf);

#include "atfuncs.h"

int
fstatat(int fd, const char *__restrict path, struct stat *buf, int flag)
{
  int ret;
  ATFUNC_VAR(fd, path);

  EINVAL_IF(flag & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (flag & AT_SYMLINK_NOFOLLOW) {
    ret = lstat(ATFUNC_PATH, buf);
  } else {
    ret = stat(ATFUNC_PATH, buf);
  }
  ATFUNC_FINISH;
  return ret;
}

int
fstatat$INODE64(int fd, const char *__restrict path,
                struct stat64 *buf, int flag)
{
  int ret;
  ATFUNC_VAR(fd, path);

  EINVAL_IF(flag & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (flag & AT_SYMLINK_NOFOLLOW) {
    ret = lstat$INODE64(ATFUNC_PATH, buf);
  } else {
    ret = stat$INODE64(ATFUNC_PATH, buf);
  }
  ATFUNC_FINISH;
  return ret;
}

#if __MPLS_HAVE_STAT64

/*
 * The fstatat64 function is not expected to be accessed directly (though many
 * system libraries provide it as a convenience synonym for fstatat$INODE64),
 * so no SDK provides a prototype for it.  We do so here.
 */

extern int fstatat64(int fd, const char *__restrict path,
                     struct stat64 *buf, int flag);

int
fstatat64(int fd, const char *path, struct stat64 *buf, int flag)
{
  int ret;
  ATFUNC_VAR(fd, path);

  EINVAL_IF(flag & ~AT_SYMLINK_NOFOLLOW);
  ATFUNC_START(-1);
  if (flag & AT_SYMLINK_NOFOLLOW) {
    ret = lstat64(ATFUNC_PATH, buf);
  } else {
    ret = stat64(ATFUNC_PATH, buf);
  }
  ATFUNC_FINISH;
  return ret;
}

#endif /* __MPLS_HAVE_STAT64 */

#endif  /* __MPLS_LIB_SUPPORT_ATCALLS__ */
