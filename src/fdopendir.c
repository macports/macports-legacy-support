
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

#include <dirent.h>

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050
#define __dd_fd dd_fd
#endif /* __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050 */

/*
 * Implementation behavior largely follows these man page descriptions:
 *
 * https://www.freebsd.org/cgi/man.cgi?query=fdopendir&sektion=3
 * https://linux.die.net/man/3/fdopendir
 */

DIR *fdopendir(int dirfd) {

    /* Check dirfd here (for macos-10.4, see _ATCALL() and best_fchdir()) */

    if (dirfd < 0) {
        errno = EBADF;
        return 0;
    }

    /* Open given directory fd safely for iteration via readdir */

    DIR *dir = _ATCALL(dirfd, ".", NULL, opendir("."));
    if (!dir)
        return 0;

    /*
     * Replace underlying fd with supplied dirfd
     * A subsequent closedir() will close dirfd
     */

    (void)close(dir->__dd_fd);
    dir->__dd_fd = dirfd;

    /*
     * Rewind to the start of the directory, in case the underlying file
     * is not positioned at the start
     */

    rewinddir(dir);

    /* Close given fd on exec (as per fdopendir() docs) */

    (void)fcntl(dirfd, F_SETFD, FD_CLOEXEC);

    return dir;
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
