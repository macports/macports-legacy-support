/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

#if __MPLS_LIB_SUPPORT_PTHREAD_CHDIR__
/*
 * The pthread_[f]chdir_np() functions are available as syscalls starting
 * in 10.5, but not as functions until 10.12.  This provides the missing
 * function wrappers where needed.
 */

#define _MACPORTS_LEGACY_PTHREAD_CHDIR 1
#include <pthread.h>
#include <unistd.h>

#include <sys/syscall.h>

int
pthread_chdir_np(const char* path)
{
  return syscall(SYS___pthread_chdir, path);
}

int
pthread_fchdir_np(int fd)
{
  return syscall(SYS___pthread_fchdir, fd);
}

#endif /* __MPLS_LIB_SUPPORT_PTHREAD_CHDIR__ */

#if __MPLS_LIB_DUMMY_PTHREAD_CHDIR__
/*
 * Dummy versions of the functions, in case the client builds with a 10.5+ SDK
 * but runs on 10.4.  There's no truly appropriate choice for errno, so we
 * pick ENXIO as one that's somewhat mnemonic and not likely to occur with
 * the real functions.
 */

#include <errno.h>

int
pthread_chdir_np(const char* path)
{
  (void) path;
  errno = ENXIO;
  return -1;
}

int
pthread_fchdir_np(int fd)
{
  (void) fd;
  errno = ENXIO;
  return -1;
}

#endif /* __MPLS_LIB_DUMMY_PTHREAD_CHDIR__ */

#if __MPLS_LIB_NEED_BEST_FCHDIR__
/*
 * Provide a per-thread fchdir() for the ATCALL code if possible, but
 * fall back to the (thread-unsafe) process-level version if not (10.4).
 */

#if !__MPLS_LIB_DUMMY_PTHREAD_CHDIR__

#define _MACPORTS_LEGACY_PTHREAD_CHDIR 1
#include <pthread.h>

int
__mpls_best_fchdir(int dirfd)
{
  return pthread_fchdir_np(dirfd);
}

#else /* __MPLS_LIB_DUMMY_PTHREAD_CHDIR__ */

#include <unistd.h>

int
__mpls_best_fchdir(int dirfd)
{
/*
 * Accept dirfd == -1 (return to process cwd in __pthread_fchdir),
 * but do nothing with it.
 */
  if (dirfd == -1) return 0;
  return fchdir(dirfd);
}

#endif /* __MPLS_LIB_DUMMY_PTHREAD_CHDIR__ */

#endif /* __MPLS_LIB_NEED_BEST_FCHDIR__ */
