/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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

#if __MPLS_LIB_SUPPORT_MKOSTEMP__

/*
 * Provide mkostemp[s]().
 *
 * This uses mkstemps() initially, then modifies the flags as needed:
 *
 * The lock flags can't be modified in place, so we need to reopen the
 * file with the proper flags if either is requested.  If we do that,
 * we set O_APPEND and O_CLOEXEC as desired in the process.
 *
 * O_APPEND can be set via fcntl(..., F_SETFL, ...).
 *
 * O_CLOEXEC can be set via fcntl(..., F_SETFD, ...).
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define ALLOWED_MKOSTEMP_FLAGS (O_APPEND | O_SHLOCK | O_EXLOCK | O_CLOEXEC)

static void
safe_close(int fd)
{
  int saverr = errno;
  (void) close(fd);
  errno = saverr;
}

int
mkostemp(char *path, int oflags)
{
  return mkostemps(path, 0, oflags);
}

int
mkostemps(char *path, int slen, int oflags)
{
  int fd, fd2, flags, err;

  if (oflags & ~ALLOWED_MKOSTEMP_FLAGS) {
    errno = EINVAL;
    return -1;
  }

  fd = mkstemps(path, slen);
  if (fd < 0) return -1;

  if (oflags & (O_SHLOCK | O_EXLOCK)) {
    fd2 = open(path, oflags);
    safe_close(fd);
    return fd2;
  }

  if (oflags & O_APPEND) {
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
      safe_close(fd);
      return -1;
    }
    flags |= O_APPEND;
    err = fcntl(fd, F_SETFL, flags);
    if (err == -1) {
      safe_close(fd);
      return -1;
    }
  }

  if (oflags & O_CLOEXEC) {
    err = fcntl(fd, F_SETFD, 1);
    if (err == -1) {
      safe_close(fd);
      return -1;
    }
  }

  return fd;
}

#endif  /* __MPLS_LIB_SUPPORT_MKOSTEMP__ */
