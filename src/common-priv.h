/*-
 * Copyright (c) 2019-2021 Ken Cunningham kencu@macports.org
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

#ifndef macports_common_priv_h_
#define macports_common_priv_h_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Should be generic enough so that we don't need a global feature macro. */

#include <sys/errno.h>

#if __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__
#include <sys/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#endif /* __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__ */

#define PROTECT_ERRNO(what)  ({ int __err = (errno); what; errno = __err; })
#define ERR_ON(code, what)   if (what) { errno = (code); return -1; }

#ifndef SYS___pthread_fchdir
# define SYS___pthread_fchdir 349
#endif


#if __MP_LEGACY_SUPPORT_NEED_BEST_FCHDIR__

int best_fchdir(int dirfd);

#endif /* __MP_LEGACY_SUPPORT_NEED_BEST_FCHDIR__ */


#if __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__

#define _ATCALL(fd, p, onerr, what)                             \
    ({  typeof(what) __result;                                  \
        int oldCWD = -1;                                        \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            oldCWD = open(".", O_RDONLY);                       \
            if (best_fchdir(-1) < 0 && oldCWD != -1) {          \
                close(oldCWD); oldCWD = -1;                     \
            }                                                   \
            if (best_fchdir(fd) < 0) {                          \
                PROTECT_ERRNO(best_fchdir(oldCWD));             \
                if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD)); \
                return onerr;                                   \
            }                                                   \
        }                                                       \
        __result = (what);                                      \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            PROTECT_ERRNO(best_fchdir(oldCWD));                 \
            if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));     \
        }                                                       \
        __result;                                               \
    })

#define ATCALL(fd, p, what)  _ATCALL(fd, p, -1, what)

#endif /* __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__ */

#endif /* !defined (macports_common_priv_h_) */
