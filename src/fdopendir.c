
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

#include <dirent.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>

#ifndef SYS___pthread_fchdir
# define SYS___pthread_fchdir 349
#endif

int best_fchdir(int dirfd)
{
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
  return syscall(SYS___pthread_fchdir, dirfd);
#else
/* Tiger does not have kernel support for __pthread_fchdir, so we have to fall back to fchdir */
/* unless we can come up with a per-thread compatible implementation that works on Tiger */
  return syscall(SYS_fchdir, dirfd);
#endif
}

#define PROTECT_ERRNO(what)  ({ int __err = (errno); what; errno = __err; })

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

    if (dirfd == AT_FDCWD)
        return opendir (".");

    oldCWD = open(".", O_RDONLY);
    if (oldCWD == -1)
        return 0;

    if(best_fchdir(dirfd) < 0) {
        if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));
        return 0;
    }

    dir = opendir (".");

    if (best_fchdir(oldCWD) < 0) {
        if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));
        return 0;
    }

    if (oldCWD != -1)
        PROTECT_ERRNO(close(oldCWD));

    return dir;
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
