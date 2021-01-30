
/*-
 * Copyright (c) 2019 Ken Cunningham kencu@macports.org
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

#include <dirent.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


/*
 * Implementation behavior largely follows these man page descriptions:
 *
 * https://www.freebsd.org/cgi/man.cgi?query=fdopendir&sektion=3
 * https://linux.die.net/man/3/fdopendir
 */

DIR *fdopendir(int dirfd) {
    DIR *dir;
    struct stat st;
    int oldCWD = -1;

    if (fstat(dirfd, &st) < 0)
        return 0;

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return 0;
    }

    if (dirfd == AT_FDCWD) {
        dir = opendir (".");
        /* dirfd can be closed only upon success */
        if (dir) PROTECT_ERRNO(close(dirfd));
        return dir;
    }

    oldCWD = open(".", O_RDONLY);
    if (oldCWD == -1)
        return 0;

    if(best_fchdir(dirfd) < 0) {
        if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));
        return 0;
    }

    dir = opendir (".");

    if (best_fchdir(oldCWD) < 0) {
        if (dir) PROTECT_ERRNO(closedir(dir));
        if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));
        return 0;
    }

    if (oldCWD != -1)
        PROTECT_ERRNO(close(oldCWD));


    /*
     * FIXME -- this recently added bit makes the fdopendir tests fail on Tiger.
     * check the whole commit where it was added to make sure it is
     * doing the proper thing on all systems. Probably need more extensive tests
     * to execise the whole system more aggressively. -- kencu@macports.org
     */

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
    /* dirfd can be closed only upon success */
    if (dir && dirfd != -1) PROTECT_ERRNO(close(dirfd));
#endif

    return dir;
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
