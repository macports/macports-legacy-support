
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
#if __MP_LEGACY_SUPPORT_FDOPENDIR__

#include "common-priv.h"

#define __MP_LEGACY_SUPPORT_NO_DIRFD_MACRO

#include <stdlib.h>
#include <dirent.h>
#include <sys/errno.h>

#undef DIR


/*
 * Implementation behavior largely follows these man page descriptions:
 *
 * https://www.freebsd.org/cgi/man.cgi?query=fdopendir&sektion=3
 * https://linux.die.net/man/3/fdopendir
 *
 * On success, this function returns allocated memory that must be
 * deallocated by __mpls_closedir() (see closedir() macro in <dirent.h>).
 */

__MPLS_DIR *fdopendir(int dirfd) {

    /* Check dirfd here (for macos-10.4, see _ATCALL() and best_fchdir()) */

    if (dirfd < 0) {
        errno = EBADF;
        return 0;
    }

    /* Open the supplied directory safely */

    DIR *dir = _ATCALL(dirfd, ".", NULL, __mpls_libc_opendir("."));
    if (!dir)
        return 0;

    /* Wrap it and return it (with the supplied directory file descriptor) */

    __MPLS_DIR *mplsdir = malloc(sizeof(*mplsdir));
    if (!mplsdir) {
        (void)__mpls_libc_closedir(dir);
        errno = ENOMEM;
        return 0;
    }

    mplsdir->__mpls_dir = dir;
    mplsdir->__mpls_dirfd = dirfd;

    return mplsdir;
}

/*
 * Wrapped version of opendir() for fdopendir() compatibility
 *
 * On success, this function returns allocated memory that must be
 * deallocated by __mpls_closedir() (see closedir() macro in <dirent.h>).
 */

__MPLS_DIR *__mpls_opendir(const char *name) {

    DIR *dir = __mpls_libc_opendir(name);
    if (!dir)
        return 0;

    __MPLS_DIR *mplsdir = malloc(sizeof(*mplsdir));
    if (!mplsdir) {
        (void)__mpls_libc_closedir(dir);
        errno = ENOMEM;
        return 0;
    }

    mplsdir->__mpls_dir = dir;
    mplsdir->__mpls_dirfd = -1;

    return mplsdir;
}

/*
 * Wrapped version of closedir() for fdopendir() compatibility (see
 * closedir() macro in <dirent.h>).
 *
 * This function deallocates memory that was allocated by fdopendir() or
 * __mpls_opendir().
 */

int __mpls_closedir(__MPLS_DIR *mplsdir) {

    if (!mplsdir) {
        errno = EBADF;
        return -1;
    }

    int rc = __mpls_libc_closedir(mplsdir->__mpls_dir);

    if (mplsdir->__mpls_dirfd != -1)
        PROTECT_ERRNO(close(mplsdir->__mpls_dirfd));

    free(mplsdir);

    return rc;
}

/*
 * Wrapped version of readdir() for fdopendir() compatibility.
 */

struct dirent *__mpls_readdir(__MPLS_DIR *mplsdir) {
    return __mpls_libc_readdir(mplsdir->__mpls_dir);
}

/*
 * Wrapped version of readdir_r() for fdopendir() compatibility.
 */

int __mpls_readdir_r(__MPLS_DIR *mplsdir, struct dirent *entry, struct dirent **result) {
    return __mpls_libc_readdir_r(mplsdir->__mpls_dir, entry, result);
}

/*
 * Wrapped version of rewinddir() for fdopendir() compatibility.
 */

void __mpls_rewinddir(__MPLS_DIR *mplsdir) {
    __mpls_libc_rewinddir(mplsdir->__mpls_dir);
}

/*
 * Wrapped version of seekdir() for fdopendir() compatibility.
 */

void __mpls_seekdir(__MPLS_DIR *mplsdir, long loc) {
    __mpls_libc_seekdir(mplsdir->__mpls_dir, loc);
}

/*
 * Wrapped version of telldir() for fdopendir() compatibility.
 */

long __mpls_telldir(__MPLS_DIR *mplsdir) {
    return __mpls_libc_telldir(mplsdir->__mpls_dir);
}

/*
 * Wrapped version of dirfd() for fdopendir() compatibility because dirfd()
 * itself is already a macro (see dirfd() macro in <dirent.h>).
 */

int __mpls_dirfd(__MPLS_DIR *mplsdir) {

    /* Return the supplied directory file descriptor if there was one */

    if (mplsdir->__mpls_dirfd != -1)
        return mplsdir->__mpls_dirfd;

    /* Otherwise call the underlying dirfd() macro */

    return dirfd(mplsdir->__mpls_dir);
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
