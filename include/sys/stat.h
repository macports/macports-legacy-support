/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

#ifndef _MACPORTS_SYS_STAT_H_
#define _MACPORTS_SYS_STAT_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system sys/stat.h */
#include_next <sys/stat.h>

#if __MPLS_SDK_SUPPORT_STAT64__

/* In lieu of 10.5+ sys/_types.h + machine/_types.h: */
typedef unsigned long long __darwin_ino64_t;

/* The following section is a subset of the Apple 10.5 sys/stat.h. */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

#define __DARWIN_STRUCT_STAT64_TIMES \
	struct timespec st_atimespec;		/* time of last access */ \
	struct timespec st_mtimespec;		/* time of last data modification */ \
	struct timespec st_ctimespec;		/* time of last status change */ \
	struct timespec st_birthtimespec;	/* time of file creation(birth) */

#else /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

#define __DARWIN_STRUCT_STAT64_TIMES \
	time_t		st_atime;		/* [XSI] Time of last access */ \
	long		st_atimensec;		/* nsec of last access */ \
	time_t		st_mtime;		/* [XSI] Last data modification time */ \
	long		st_mtimensec;		/* last data modification nsec */ \
	time_t		st_ctime;		/* [XSI] Time of last status change */ \
	long		st_ctimensec;		/* nsec of last status change */ \
	time_t		st_birthtime;		/*  File creation time(birth)  */ \
	long		st_birthtimensec;	/* nsec of File creation time */

#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

/*
 * This structure is used as the second parameter to the fstat64(),
 * lstat64(), and stat64() functions, and for struct stat when
 * __DARWIN_64_BIT_INO_T is set. __DARWIN_STRUCT_STAT64 is defined
 * above, depending on whether we use struct timespec or the direct
 * components.
 *
 * This is simillar to stat except for 64bit inode number
 * number instead of 32bit ino_t and the addition of create(birth) time.
 */
#define __DARWIN_STRUCT_STAT64 { \
	dev_t		st_dev;			/* [XSI] ID of device containing file */ \
	mode_t		st_mode;		/* [XSI] Mode of file (see below) */ \
	nlink_t		st_nlink;		/* [XSI] Number of hard links */ \
	__darwin_ino64_t st_ino;		/* [XSI] File serial number */ \
	uid_t		st_uid;			/* [XSI] User ID of the file */ \
	gid_t		st_gid;			/* [XSI] Group ID of the file */ \
	dev_t		st_rdev;		/* [XSI] Device ID */ \
	__DARWIN_STRUCT_STAT64_TIMES \
	off_t		st_size;		/* [XSI] file size, in bytes */ \
	blkcnt_t	st_blocks;		/* [XSI] blocks allocated for file */ \
	blksize_t	st_blksize;		/* [XSI] optimal blocksize for I/O */ \
	__uint32_t	st_flags;		/* user defined flags for file */ \
	__uint32_t	st_gen;			/* file generation number */ \
	__int32_t	st_lspare;		/* RESERVED: DO NOT USE! */ \
	__int64_t	st_qspare[2];		/* RESERVED: DO NOT USE! */ \
}

/* End of first grab from Apple 10.5 sys/stat.h */

/* More from Apple 10.5 sys/stat.h (slightly tweaked) */

#if !defined(_POSIX_C_SOURCE) \
    || (defined(_DARWIN_C_SOURCE) && __MPLS_SDK_MAJOR >= 1050)

struct stat64 __DARWIN_STRUCT_STAT64;

int	fstatx64_np(int, struct stat64 *, filesec_t);
int	lstatx64_np(const char *, struct stat64 *, filesec_t);
int	statx64_np(const char *, struct stat64 *, filesec_t);
int	fstat64(int, struct stat64 *);
int	lstat64(const char *, struct stat64 *);
int	stat64(const char *, struct stat64 *);

#endif /* (!_POSIX_C_SOURCE || (_DARWIN_C_SOURCE && >10.4)) */

/* End of additional grabs from Apple 10.5 sys/stat.h */

#endif /* __MPLS_SDK_SUPPORT_STAT64__ */

/* Set up condition for having "struct stat64" defined by the SDK. */
#if ((!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)) \
     && (!defined(__DARWIN_ONLY_64_BIT_INO_T) || !__DARWIN_ONLY_64_BIT_INO_T))
#define __MPLS_HAVE_STAT64 1
#else
#define __MPLS_HAVE_STAT64 0
#endif

#if __DARWIN_C_LEVEL >= 200809L

#if __MPLS_SDK_SUPPORT_UTIMENSAT__

#define UTIME_NOW -1
#define UTIME_OMIT -2

__MP__BEGIN_DECLS

extern int futimens(int fd, const struct timespec _times_in[2]);
extern int utimensat(int fd, const char *path,
                     const struct timespec _times_in[2], int flags);

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_UTIMENSAT__ */

#if __MPLS_SDK_SUPPORT_ATCALLS__

__MP__BEGIN_DECLS

extern int fchmodat(int fd, const char *path, mode_t mode, int flag);
extern int fstatat(int fd, const char *path,
                   struct stat *buf, int flag) __DARWIN_INODE64(fstatat);

/*
 * Some versions of this header have included a prototype for fstatat64().
 * This is inappropriate, since no SDK has ever directly provided that
 * function.  The intent is that any use of 64-bit-inodes should be
 * via symbol versioning, though many versions of the system library
 * have made fstatat64 available as a convenience alias for fstatat$INODE64.
 *
 * For consistency, we don't provide fstatat64() here.  All our own
 * internal references provide their own prototypes.
 */

extern int mkdirat(int fd, const char *path, mode_t mode);

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_ATCALLS__ */

#endif /* __DARWIN_C_LEVEL >= 200809L */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

#if __MPLS_SDK_SUPPORT_LCHMOD__

__MP__BEGIN_DECLS

extern int lchmod(const char *, mode_t);

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_LCHMOD__ */

#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#endif /* _MACPORTS_SYS_STAT_H_ */
