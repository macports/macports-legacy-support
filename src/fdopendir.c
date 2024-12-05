/*
 * Copyright (c) 2019
 * Copyright (C) 2023 raf <raf@raf.org>, Tavian Barnes <tavianator@tavianator.com>
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

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/*
 * Implementation behavior largely follows these man page descriptions:
 *
 * https://www.freebsd.org/cgi/man.cgi?query=fdopendir&sektion=3
 * https://linux.die.net/man/3/fdopendir
 */

#if __MPLS_LIB_SUPPORT_ATCALLS__

/*
 * Set up to use ino32 variants where possible.  This results in generating
 * the "unadorned" names, which we augment with explicit suffixes where
 * needed.  Similarly, we use the non-POSIX form in 32-bit builds.
 *
 * The ino32 variants are known to be unavailable in arm64 builds, but are
 * available in all OS versions that need our fdopendir().
 *
 * Although the struct dirent is formatted differently for ino32 and ino64,
 * we never directly reference it here.  The only difference in DIR is the
 * format of the private struct _telldir pointed to by the __dd_td field,
 * which we also never reference here.  Hence, we don't care which variant
 * we're using, except for passing the choice through to the underlying
 * functions.
 *
 * In the case of struct stat, we provide our own ino64 variant where needed.
 */

#define _DARWIN_NO_64_BIT_INODE 1
#if !__MPLS_64BIT
#define _NONSTD_SOURCE
#endif

#include <dirent.h>
#include <stddef.h>

#include <sys/stat.h>

#include "atcalls.h"

#if __MPLS_SDK_MAJOR < 1050
#define __dd_fd dd_fd
#endif /* __MPLS_SDK_MAJOR < 1050
*/

/* Make sure we have "struct sta64" */
#if !__MPLS_HAVE_STAT64
struct stat64 __DARWIN_STRUCT_STAT64;
#endif /* !__MPLS_HAVE_STAT64 */

/* Universal stat buffer, accommodating both formats */
union stat_u {
  struct stat s;
  struct stat64 s64;
};

/* Type declarations for external functions */
typedef int (stat_fn_t)(int fd, struct stat *buf);
typedef int (stat64_fn_t)(int fd, struct stat64 *buf);
typedef DIR * (opn_fn_t)(const char *dirname);
typedef void (rwd_fn_t)(DIR *dirp);

/* Structure for per-variant dispatch table */
typedef struct funcs_s {
  stat_fn_t *do_fstat;
  stat64_fn_t *do_fstat64;
  opn_fn_t *do_open;
  rwd_fn_t *do_rewind;
} funcs_t;

/* Common function used by all variants - controlled by a dispatch table */
static DIR *
fdopendir_internal(int fd, const funcs_t *funcs) {
  DIR *dir;
  int err;
  mode_t mode;
  union stat_u stbuf;

  /* Do the appropriate fstat() on the supplied fd */
  if (funcs->do_fstat64) {
    err = (*funcs->do_fstat64)(fd, &stbuf.s64);
    mode = stbuf.s64.st_mode;
  } else if (funcs->do_fstat) {
    err = (*funcs->do_fstat)(fd, &stbuf.s);
    mode = stbuf.s.st_mode;
  } else {
    errno = EINVAL;  /* Should be impossible */
    return NULL;
  }

  /* Fail if fd isn't a valid open fd */
  if (err < 0) {
    return NULL;
  }

  /* Fail if fd isn't a directory */
  if (!S_ISDIR(mode)) {
    errno = ENOTDIR;
    return NULL;
  }

  /* Open given directory fd safely for iteration via readdir */

  dir = _ATCALL(fd, ".", NULL, (*funcs->do_open)("."));
  if (!dir) {
    return NULL;
  }

  /*
   * Replace underlying fd with supplied fd
   * A subsequent closedir() will close fd
   */

  (void)close(dir->__dd_fd);
  dir->__dd_fd = fd;

  /*
   * Rewind to the start of the directory, in case the underlying file
   * is not positioned at the start
   */

  (*funcs->do_rewind)(dir);

  /* Close given fd on exec (as per fdopendir() docs) */

  (void)fcntl(fd, F_SETFD, FD_CLOEXEC);

  return dir;
}

/*
 * Now to handle all the variants:
 *
 * Define a macro listing all variants supported by the OS/arch.
 * 10.4 lacks the INODE64 variants.
 * 64-bit builds lack the UNIX2003 variants.
 */

#if __MPLS_TARGET_OSVER < 1050
#if !__MPLS_64BIT

#define ALL_VARIANTS \
  VARIANT_ENT(basic,,,) \
  VARIANT_ENT(posix,,$UNIX2003,)

#else /* __MPLS_64BIT */

#define ALL_VARIANTS \
  VARIANT_ENT(basic,,,)

#endif /* __MPLS_64BIT */
#else /* __MPLS_TARGET_OSVER >= 1050 */
#if !__MPLS_64BIT

#define ALL_VARIANTS \
  VARIANT_ENT(basic,,,) \
  VARIANT_ENT(posix,,$UNIX2003,) \
  VARIANT_ENT(ino64,$INODE64,$UNIX2003,64)

#else /* __MPLS_64BIT */

#define ALL_VARIANTS \
  VARIANT_ENT(basic,,,) \
  VARIANT_ENT(ino64,$INODE64,,64)

#endif /* __MPLS_64BIT */
#endif /* __MPLS_TARGET_OSVER >= 1050 */

/* Declare all called functions with appropriate suffixes. */
/* The "basic" case is redundant but serves as an error check. */
#define VARIANT_ENT(name,isfx,usfx,is64) \
stat##is64##_fn_t fstat##isfx; \
opn_fn_t opendir##isfx##usfx; \
rwd_fn_t rewinddir##isfx##usfx;
ALL_VARIANTS
#undef VARIANT_ENT

/* Generate dispatch tables. */
/*
 * Since compilers aren't smart enough to avoid complaining about mismatched
 * types from values made irrelevant by compile-time constants, we include
 * technically incorrect casts to shut them up.
 */
#define VARIANT_ENT(name,isfx,usfx,is64) \
static const funcs_t name = { \
  .do_fstat = 0##is64 ? NULL : (stat_fn_t *) &fstat##isfx, \
  .do_fstat64 = 0##is64 ? (stat64_fn_t *) &fstat##isfx : NULL, \
  .do_open = &opendir##isfx##usfx, \
  .do_rewind = &rewinddir##isfx##usfx, \
};
ALL_VARIANTS
#undef VARIANT_ENT

/* Now generate the functions. */
#define VARIANT_ENT(name,isfx,usfx,is64) \
DIR * \
fdopendir##isfx##usfx(int fd) \
{ \
  return fdopendir_internal(fd, &name); \
}
ALL_VARIANTS
#undef VARIANT_ENT

#endif /* __MPLS_LIB_SUPPORT_ATCALLS__ */
