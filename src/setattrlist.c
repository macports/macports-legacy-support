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

#if  __MPLS_LIB_FIX_SETATTRLIST__

/*
 * Versions of [f]setattrlist() on 10.5-10.7 have a bug where if no
 * attributes are requested to be set and the attribute buffer has zero
 * length, then it fails with ENOMEM instead of being a successful NOP.
 * This provides a fix for that.
 *
 * NOTE: This needs to be a separate source from fxetattrlist, due to
 * the handling of the variant suffixes.
 */

/* Keep unistd from defining any version of setattrlist directly */
#define setattrlist __os_setattrlist

#include <errno.h>
#include <unistd.h>

#include <sys/attr.h>

/* Now undo our macro kludge */
#undef setattrlist

#include "util.h"

#ifdef __LP64__
typedef unsigned int attrlist_opts_t;
#else /* !__LP64__ */
typedef unsigned long attrlist_opts_t;
#endif /* !__LP64__ */

/* In relevant case, turn error into success */
static int
fix_ret(int ret, int saverr, struct attrlist *al, size_t bufsiz)
{
  if (bufsiz == 0
      && al->commonattr == 0
      && (al->volattr & ~ATTR_VOL_INFO) == 0
      && al->dirattr == 0
      && al->fileattr == 0
      && al->forkattr == 0) {
    errno = saverr;
    return 0;
  }
  return ret;
}

int
setattrlist(const char *path, void *attrList, void *attrBuf,
            size_t attrBufSize, attrlist_opts_t options)
{
  int ret, saverr = errno;
  GET_OS_FUNC(setattrlist)

  /* Try the call - if it's not the possible bug, just pass the result. */
  ret = (*os_setattrlist)(path, attrList, attrBuf, attrBufSize, options);
  if (!ret || errno != ENOMEM) return ret;

  /* Else apply the fix if appropriate. */
  return fix_ret(ret, saverr, attrList, attrBufSize);
}

#ifndef __LP64__
/*
 * 32-bit builds may have two variants.  Since this code doesn't apply to
 * 10.4, we don't need the fallback to the "unadorned" variant.
 */

int
setattrlist$UNIX2003(const char *path, void *attrList, void *attrBuf,
                     size_t attrBufSize, attrlist_opts_t options)
{
  int ret, saverr = errno;
  GET_OS_ALT_FUNC(setattrlist,$UNIX2003)

  /* Try the call - if it's not the possible bug, just pass the result. */
  ret = (*os_setattrlist)(path, attrList, attrBuf, attrBufSize, options);
  if (!ret || errno != ENOMEM) return ret;

  /* Else apply the fix if appropriate. */
  return fix_ret(ret, saverr, attrList, attrBufSize);
}

#endif  /* !__LP64__ */

#if !__MPLS_LIB_SUPPORT_FXETATTRLIST__
/*
 * If we're not providing a missing fsetattrlist(), then we need to
 * wrap it here to apply the same fix.
 */

int
fsetattrlist(int fd, void *attrList, void *attrBuf,
            size_t attrBufSize, attrlist_opts_t options)
{
  int ret, saverr = errno;
  GET_OS_FUNC(fsetattrlist)

  /* Try the call - if it's not the possible bug, just pass the result. */
  ret = (*os_fsetattrlist)(fd, attrList, attrBuf, attrBufSize, options);
  if (!ret || errno != ENOMEM) return ret;

  /* Else apply the fix if appropriate. */
  return fix_ret(ret, saverr, attrList, attrBufSize);
}

#endif  /* !__MPLS_LIB_SUPPORT_FXETATTRLIST__ */

#endif  /* __MPLS_LIB_FIX_SETATTRLIST__ */
