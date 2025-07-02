/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

#if __MPLS_LIB_FIX_TIGER_PPC64__

/*
 * In 10.4 ppc64, there is a bug in fcntl() which doesn't handle 64-bit
 * addresses correctly for F_GETPATH.  To work around this, we need to
 * use a buffer in the low 4GiB of memory for the temporary path.  Since
 * 64-bit builds on 10.4 don't bother to steer clear of the low 4GiB,
 * this can be accomplished with a static buffer, but we need to lock it
 * against thread collisions.
 *
 * Wrapping fcntl() is made drastically more complicated by the fact that
 * it's a variadic function and there's no vfcntl().  So we try to get
 * the argument type correct based on the 'cmd' argument.  We use
 * void * for all pointers (except the one we operate on), since the pointers
 * themselves aren't type-specific.
 *
 * Further complicating things is that we don't want to crash on a genuinely
 * bad pointer, so we need to check it explicitly.
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>

#include "compiler.h"
#include "util.h"

static pthread_mutex_t path_lock = PTHREAD_MUTEX_INITIALIZER;
static char pathbuf[MAXPATHLEN];

typedef int (fcntl_fn_t)(int fildes, int cmd, ...);

int
fcntl(int fildes, int cmd, ...)
{
  va_list ap;
  union arg_u {
    int num;
    pid_t pid;
    off_t off;
    void *ptr;
    char *str;
  } arg;

  int ret;
  static fcntl_fn_t *os_fcntl = NULL;

  if (MPLS_SLOWPATH(!os_fcntl)) {
    os_fcntl = dlsym(RTLD_NEXT, "fcntl");
    /* Something's badly broken if this fails */
    if (!os_fcntl) {
        abort();
    }
  }

  va_start(ap, cmd);
  switch (cmd) {

  case F_GETFD:
  case F_GETFL:
  case F_GETOWN:
  case F_FULLFSYNC:
  case F_FREEZE_FS:
  case F_THAW_FS:
    va_end(ap);
    return (*os_fcntl)(fildes, cmd);

  case F_DUPFD:
  case F_SETFD:
  case F_SETFL:
  case F_RDAHEAD:
  case F_NOCACHE:
    arg.num = va_arg(ap, int);
    va_end(ap);
    return (*os_fcntl)(fildes, cmd, arg.num);

  case F_SETOWN:
    arg.pid = va_arg(ap, pid_t);
    va_end(ap);
    return (*os_fcntl)(fildes, cmd, arg.pid);

  case F_SETSIZE:
    arg.off = va_arg(ap, off_t);
    va_end(ap);
    return (*os_fcntl)(fildes, cmd, arg.off);

  case F_PREALLOCATE:
  case F_READBOOTSTRAP:
  case F_WRITEBOOTSTRAP:
  case F_LOG2PHYS:
  case F_PATHPKG_CHECK:
    arg.ptr = va_arg(ap, void *);
    va_end(ap);
    return (*os_fcntl)(fildes, cmd, arg.ptr);

  case F_GETPATH:
    arg.str = va_arg(ap, char *);
    va_end(ap);
    ret = (*os_fcntl)(fildes, cmd, arg.str);
    if (ret != -1 || errno != EFAULT) return ret;
    break;

  default:
    arg.ptr = va_arg(ap, void *);
    va_end(ap);
    return (*os_fcntl)(fildes, cmd, arg.ptr);
  }

  /* Here when F_GETPATH gets EFAULT */

  /* If not a 64-bit issue, just punt (probably genuine bad adr). */
  if ((uint64_t) arg.ptr < (1ULL << 32)) return -1;

  /* Now do a correct access check on the result buffer, and fail if bad */
  if (__mpls_check_access(arg.ptr, MAXPATHLEN, VM_PROT_WRITE, NULL)) {
    errno = EFAULT;
    return -1;
  }

  /* All OK, now just use our local buffer and copy it (with locking). */
  if (pthread_mutex_lock(&path_lock)) return -1;
  if ((ret = (*os_fcntl)(fildes, cmd, pathbuf))) {
    (void) pthread_mutex_unlock(&path_lock);
    return ret;
  }
  memcpy(arg.ptr, pathbuf, MAXPATHLEN);
  return (errno = pthread_mutex_unlock(&path_lock)) ? -1 : 0;
}

#endif /* __MPLS_LIB_FIX_TIGER_PPC64__ */
