
/*
 * Copyright (c) 2019
 * Copyright (c) 2023 raf <raf@raf.org>, Tavian Barnes <tavianator@tavianator.com>
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

/* Include the primary system dirent.h */
#include_next <dirent.h>

/* fdopendir */
#if __MP_LEGACY_SUPPORT_FDOPENDIR__

/* Wrapper struct for DIR */
typedef struct __MP_LEGACY_SUPPORT_DIR __MP_LEGACY_SUPPORT_DIR;
struct __MP_LEGACY_SUPPORT_DIR {
    DIR *__mpls_dir;
    int __mpls_dirfd;
};
#define DIR __MP_LEGACY_SUPPORT_DIR

__MP__BEGIN_DECLS

#ifndef __DARWIN_ALIAS_I
extern DIR *fdopendir(int fd) __DARWIN_ALIAS(fdopendir);
#else
extern DIR *fdopendir(int fd) __DARWIN_ALIAS_I(fdopendir);
#endif

/* Wrapper functions/macros to support fdopendir */
extern DIR *__mpls_opendir(const char *name);
extern int __mpls_closedir(DIR *dir);
extern int __mpls_dirfd(DIR *dir);

#define opendir(name)      __mpls_opendir(name)
#define closedir(dir)      __mpls_closedir(dir)

#ifndef __MP_LEGACY_SUPPORT_NO_DIRFD_MACRO
#undef dirfd
#define dirfd(dir)         __mpls_dirfd(dir)
#endif

static inline struct dirent *__mpls_readdir(DIR *dir) {
    return readdir(dir->__mpls_dir);
}

static inline int __mpls_readdir_r(DIR *dir, struct dirent *entry, struct dirent **result) {
    return readdir_r(dir->__mpls_dir, entry, result);
}

static inline void __mpls_rewinddir(DIR *dir) {
    rewinddir(dir->__mpls_dir);
}

static inline void __mpls_seekdir(DIR *dir, long loc) {
    seekdir(dir->__mpls_dir, loc);
}

static inline long __mpls_telldir(DIR *dir) {
    return telldir(dir->__mpls_dir);
}

#define readdir __mpls_readdir
#define readdir_r __mpls_readdir_r
#define rewinddir __mpls_rewinddir
#define seekdir __mpls_seekdir
#define telldir __mpls_telldir

__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */

#endif /* _MACPORTS_DIRENT_H_ */
