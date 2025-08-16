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

#if __MPLS_LIB_SUPPORT_FXETATTRLIST__

/*
 * This provides implementations of fgetattrlist() and fsetattrlist(), by
 * using F_GETPATH to obtain the file path and then using the "non-f"
 * versions.
 *
 * The former implementations included special handling for the "null" case
 * (where no attributes are requested), but newly added tests for those
 * cases indicates that such handling was unnecessary, and it has been
 * removed.
 *
 * Under 10.4 ppc64, F_GETPATH fails to handle 64-bit addressing correctly,
 * and the OS version would fail with the stack-based buffers used here.
 * Our fcntl() wrapper handles this case, at the expense of additional
 * overhead.  This could be mostly avoided by using a low-memory static
 * path buffer here (with appropriate locking), but these functions are
 * probably sufficiently rare and sufficiently expensive in their own right
 * that such handling wouldn't be worthwhile.  Hence, we just use the
 * normally appropriate stack-based buffers, and let the fcntl() wrapper
 * fix the 10.4 ppc64 case.
 *
 * NOTE: This needs to be a separate source from setattrlist, due to
 * the handling of the variant suffixes.
 *
 * Also note that, if we're not supplying missing functions here, then
 * a wrapper for fsetattrlist() may be provided by setattrlist.
 */

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/attr.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/errno.h>

#ifdef __LP64__
typedef unsigned int attrlist_opts_t;
#else /* !__LP64__ */
typedef unsigned long attrlist_opts_t;
#endif /* !__LP64__ */

int
fgetattrlist(int fd, void *attrList, void *attrBuf,
         size_t attrBufSize, attrlist_opts_t options)
{
  int ret;
  char fpath[MAXPATHLEN];

  if ((ret = fcntl(fd, F_GETPATH, fpath))) return ret;

  return getattrlist(fpath, attrList, attrBuf, attrBufSize, options);
}

int
fsetattrlist(int fd, void *attrList, void *attrBuf,
         size_t attrBufSize, attrlist_opts_t options)
{
  int ret;
  char fpath[MAXPATHLEN];

  if ((ret = fcntl(fd, F_GETPATH, fpath))) return ret;

  return setattrlist(fpath, attrList, attrBuf, attrBufSize, options);
}

#endif  /* __MPLS_LIB_SUPPORT_FXETATTRLIST__ */
