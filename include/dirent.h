
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

#ifndef _MACPORTS_DIRENT_H_
#define _MACPORTS_DIRENT_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Alter function names declared by <dirent.h> to get them out of the way */
/* Note: These renamed names are non-functional */
#if __MP_LEGACY_SUPPORT_FDOPENDIR__
#define opendir __mpls_renamed_libc_opendir
#define closedir __mpls_renamed_libc_closedir
#define readdir __mpls_renamed_libc_readdir
#define readdir_r __mpls_renamed_libc_readdir_r
#define rewinddir __mpls_renamed_libc_rewinddir
#define seekdir __mpls_renamed_libc_seekdir
#define telldir __mpls_renamed_libc_telldir
#endif

/* Include the primary system dirent.h */
#include_next <dirent.h>

/* Remove the above macros to make way for the declarations below */
#if __MP_LEGACY_SUPPORT_FDOPENDIR__
#undef opendir
#undef closedir
#undef readdir
#undef readdir_r
#undef rewinddir
#undef seekdir
#undef telldir

/* Fallback to __DARWIN_ALIAS if the other variants are not defined (?) */
/* Note: I don't know if this makes sense */
#ifdef __DARWIN_ALIAS_I
#  define __MPLS_ALIAS_I(sym) __DARWIN_ALIAS_I(sym)
#else
#  define __MPLS_ALIAS_I(sym) __DARWIN_ALIAS(sym)
#endif

#ifdef __DARWIN_INODE64
#  define __MPLS_INODE64(sym) __DARWIN_INODE64(sym)
#else
#  define __MPLS_INODE64(sym) __DARWIN_ALIAS(sym)
#endif

/* Declare alternative names for the underlying functions for use by the wrappers */
/* Note: Each __MPLS_ALIAS* macro must match the corresponding __DARWIN_ALIAS* in system <dirent.h> */
DIR *__mpls_libc_opendir(const char *name) __MPLS_ALIAS_I(opendir);
int __mpls_libc_closedir(DIR *dir) __MPLS_ALIAS(closedir);
struct dirent *__mpls_libc_readdir(DIR *dir) __MPLS_INODE64(readdir);
int __mpls_libc_readdir_r(DIR *dir, struct dirent *entry, struct dirent **result) __MPLS_INODE64(readdir_r);
void __mpls_libc_rewinddir(DIR *dir) __MPLS_ALIAS_I(rewinddir);
void __mpls_libc_seekdir(DIR *dir, long loc) __MPLS_ALIAS_I(seekdir);
long __mpls_libc_telldir(DIR *dir) __MPLS_ALIAS_I(telldir);
#endif

/* fdopendir */
#if __MP_LEGACY_SUPPORT_FDOPENDIR__

/* Wrapper struct for DIR */
typedef struct __MPLS_DIR __MPLS_DIR;
struct __MPLS_DIR {
    DIR *__mpls_dir;
    int __mpls_dirfd;
};

#define DIR __MPLS_DIR

__MP__BEGIN_DECLS

extern DIR *fdopendir(int fd) __MPLS_ALIAS_I(fdopendir);

/* Wrapper functions to support fdopendir */
extern DIR *__mpls_opendir(const char *name);
extern int __mpls_closedir(DIR *dir);
extern struct dirent *__mpls_readdir(DIR *dir);
extern int __mpls_readdir_r(DIR *dir, struct dirent *entry, struct dirent **result);
extern void __mpls_rewinddir(DIR *dir);
extern void __mpls_seekdir(DIR *dir, long loc);
extern long __mpls_telldir(DIR *dir);
extern int __mpls_dirfd(DIR *dir);

/* Define the standard names to refer to LegacySupport's wrappers (via asm renaming) */
DIR *opendir(const char *name) __asm("___mpls_opendir");
int closedir(DIR *dir) __asm("___mpls_closedir");
struct dirent *readdir(DIR *dir) __asm("___mpls_readdir");
int readdir_r(DIR *dir, struct dirent *entry, struct dirent **result) __asm("___mpls_readdir_r");
void rewinddir(DIR *dir) __asm("___mpls_rewinddir");
void seekdir(DIR *dir, long loc) __asm("___mpls_seekdir");
long telldir(DIR *dir) __asm("___mpls_telldir");

#ifndef __MP_LEGACY_SUPPORT_NO_DIRFD_MACRO
#undef dirfd
#define dirfd(dir) __mpls_dirfd(dir)
#endif

__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */

#endif /* _MACPORTS_DIRENT_H_ */
