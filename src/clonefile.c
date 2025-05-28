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

#if __MPLS_LIB_SUPPORT_CLONEFILE__

/*
 * This provides degenerate implementations of *clonefile*() which always
 * fail, since the real versions only work on APFS, and no OS version that
 * uses this code supports APFS.
 *
 * At present, these functions always fail immediately with ENOTSUP, though
 * real implementations might fail in other ways when given bad arguments.
 */

#include <errno.h>

#include <sys/clonefile.h>
#include <sys/fcntl.h>

int
clonefile(const char *src, const char *dst, uint32_t flags)
{
  return clonefileat(AT_FDCWD, src, AT_FDCWD, dst, flags);
}

int
clonefileat(int src_dirfd, const char *src,
            int dst_dirfd, const char *dst, uint32_t flags)
{
  (void) src_dirfd; (void) src; (void) dst_dirfd; (void) dst; (void) flags;

  errno = ENOTSUP;
  return -1;
}

int
fclonefileat(int srcfd, int dst_dirfd, const char *dst, uint32_t flags)
{
  (void) srcfd; (void) dst_dirfd; (void) dst; (void) flags;

  errno = ENOTSUP;
  return -1;
}

#endif /* __MPLS_LIB_SUPPORT_CLONEFILE__ */
