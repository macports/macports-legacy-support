/*
 * Copyright (c) 2019
 * Copyright (c) 2023 raf <raf@raf.org>
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

#if __MPLS_LIB_NEED_BEST_FCHDIR__

#include "common-priv.h"

#include <unistd.h>
#include <sys/syscall.h>

int best_fchdir(int dirfd)
{
#if __MPLS_TARGET_OSVER >= 1050
  return syscall(SYS___pthread_fchdir, dirfd);
#else
/* Tiger does not have kernel support for __pthread_fchdir, so we have to fall back to fchdir */
/* unless we can come up with a per-thread compatible implementation that works on Tiger. */
/* Accept dirfd == -1 (which is meaningful for __pthread_fchdir) but do nothing with it. */
  if (dirfd == -1)
    return 0;
  return syscall(SYS_fchdir, dirfd);
#endif
}

#endif /* __MPLS_LIB_NEED_BEST_FCHDIR__ */
