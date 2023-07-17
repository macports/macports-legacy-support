
/*
 * Copyright (c) 2019
 * Copyright (c) 2023 raf <raf@raf.org>, Tavian Barnes <tavianator@tavianator.com>
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
#undef opendir
#undef closedir


/*
 * Implementation behavior largely follows these man page descriptions:
 *
 * https://www.freebsd.org/cgi/man.cgi?query=fdopendir&sektion=3
 * https://linux.die.net/man/3/fdopendir
 *
 * On success, this function returns allocated memory that must be
 * deallocated by __mpls_closedir() (see closedir() macro in <dirent.h>).
 */

__MP_LEGACY_SUPPORT_DIR *fdopendir(int dirfd) {

    /* Check dirfd here (for macos-10.4, see _ATCALL() and best_fchdir()) */

    if (dirfd != AT_FDCWD && dirfd < 0) {
        errno = EBADF;
        return 0;
    }

    /* Open the supplied directory safely */

    DIR *dir = _ATCALL(dirfd, ".", NULL, opendir("."));
    if (!dir)
        return 0;

    /* Wrap it and return it (with the supplied directory file descriptor) */

    __MP_LEGACY_SUPPORT_DIR *mplsdir = malloc(sizeof(*mplsdir));
    if (!mplsdir) {
        (void)closedir(dir);
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

__MP_LEGACY_SUPPORT_DIR *__mpls_opendir(const char *name) {

    DIR *dir = opendir(name);
    if (!dir)
        return 0;

    __MP_LEGACY_SUPPORT_DIR *mplsdir = malloc(sizeof(*mplsdir));
    if (!mplsdir) {
        (void)closedir(dir);
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

int __mpls_closedir(__MP_LEGACY_SUPPORT_DIR *mplsdir) {

    if (!mplsdir) {
        errno = EBADF;
        return -1;
    }

    int rc = closedir(mplsdir->__mpls_dir);

    if (mplsdir->__mpls_dirfd != AT_FDCWD && mplsdir->__mpls_dirfd != -1)
        PROTECT_ERRNO(close(mplsdir->__mpls_dirfd));

    free(mplsdir);

    return rc;
}

/*
 * Wrapped version of dirfd() for fdopendir() compatibility because dirfd()
 * itself is already a macro (see dirfd() macro in <dirent.h>).
 */

int __mpls_dirfd(__MP_LEGACY_SUPPORT_DIR *mplsdir) {

    /* Return the supplied directory file descriptor if there was one */

    if (mplsdir->__mpls_dirfd != AT_FDCWD && mplsdir->__mpls_dirfd != -1)
        return mplsdir->__mpls_dirfd;

    /* Otherwise call the underlying dirfd() macro */

    return dirfd(mplsdir->__mpls_dir);
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
