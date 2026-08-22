/*
 * Copyright (c) 2026
 * from an example posted in Apple Developer Support
 * https://forums.developer.apple.com/thread/103162
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
#if __MPLS_LIB_SUPPORT_FSGETPATH__

/*
 * This provides an implementation of fsgetpath() that's valid for
 * >= 10.6, by using the existing syscall (present without the function
 * wrapper since 10.6).
 *
 * In the aforementioned thread, Apple discouraged this approach, but that's
 * because Apple always discourages the direct use of syscalls, since they
 * don't want to guarantee compatibility at that level.  However, this is only
 * used for OS versions <10.13, where it's known to work and is essentially
 * frozen for all time, making Apple's objection irrelevant in practice.
 *
 * In the < 10.6 case, we simply return ENOTSUP, since no real implementation
 * has been devised.
 */

#include <unistd.h>

#include <sys/fsgetpath.h>
#include <sys/syscall.h>
#include <sys/types.h>

#if __MPLS_TARGET_OSVER >= 1060

ssize_t
fsgetpath(char *buf, size_t buflen, fsid_t *fsid, uint64_t obj_id)
{
  return syscall(SYS_fsgetpath, buf, buflen, fsid, obj_id);
}

#else  /* __MPLS_TARGET_OSVER < 1060 */

#include <errno.h>

ssize_t
fsgetpath(char *buf, size_t buflen, fsid_t *fsid, uint64_t obj_id)
{
  (void) buf; (void) buflen; (void) fsid; (void) obj_id;

  errno = ENOTSUP;
  return -1;
}

#endif /* __MPLS_TARGET_OSVER < 1060 */

/*
 * This file had previously included a (disabled) alternate implementation
 * as recommended by Apple in the aforementioned thread.  But it relies
 * on the ATTR_CMN_FULLPATH attribute, which doesn't exist prior to 10.6,
 * and thus provides no actual benefit.
 *
 * Interested parties can find it in the git history.
 */

#endif /* __MPLS_LIB_SUPPORT_FSGETPATH__ */
