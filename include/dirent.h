/*
 * Copyright (c) 2019
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

#ifndef _MACPORTS_DIRENT_H_
#define _MACPORTS_DIRENT_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system dirent.h */
#include_next <dirent.h>

/* Additional functionality provided by:
 * POSIX.1-2008
 */
#if __DARWIN_C_LEVEL >= 200809L

/* fdopendir */
#if __MPLS_SDK_SUPPORT_FDOPENDIR__

__MP__BEGIN_DECLS

#ifndef __DARWIN_ALIAS_I
extern DIR *fdopendir(int fd) __DARWIN_ALIAS(fdopendir);
#else
extern DIR *fdopendir(int fd) __DARWIN_ALIAS_I(fdopendir);
#endif

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_FDOPENDIR__ */

/* new signature for scandir and alphasort (optionally) */
#if __MPLS_SDK_SUPPORT_NEW_SCANDIR__ \
    && defined(_MACPORTS_LEGACY_COMPATIBLE_SCANDIR) \
    && _MACPORTS_LEGACY_COMPATIBLE_SCANDIR

/*
 * Here we provide compatibility wrappers so that scandir and alphasort
 * references based on the new function signatures can be used with the
 * old versions of the functions.  To avoid breaking code expecting the
 * normal behavior, these wrappers are only introduced conditionally,
 * based on _MACPORTS_LEGACY_COMPATIBLE_SCANDIR being nonzero.
 *
 * The wrapper functions are defined as inlines, and then macros are
 * defined to make them replace the normal functions.  Since the sole
 * effect of the wrappers is to apply some casts for pointer-type
 * compatibility, it's unlikely that any actual executable code is added.
 * As inline functions, it's still legal to take pointers to them, in
 * which case the compiler will generate local callable instances.
 *
 * The underlying functions have the DARWIN_ALIAS treatment, to handle
 * optional INODE64 behavior.  The inline functions don't need that, since
 * they simply wrap the calls to the underlying functions, which are then
 * directed as appropriate.  In the function pointer case, any local
 * callable instance is private to the compilation unit, and need only
 * handle the INODE64 case in effect there.
 */

__MP__BEGIN_DECLS

static inline int
__mpls_alphasort(const struct dirent **d1, const struct dirent **d2)
{
  /*
   * The casts here are currently unnecessary, but could be needed in
   * the future if compilers get pickier.
   */
  return alphasort((const void *) d1, (const void *) d2);
}

static inline int
__mpls_scandir(const char *dirname, struct dirent ***namelist,
               int (*select)(const struct dirent *),
               int (*compar)(const struct dirent **, const struct dirent **))
{
  return scandir(dirname, namelist,
                 (int (*)(struct dirent *)) select,
                 (int (*)(const void *, const void *)) compar);
}

__MP__END_DECLS

#define alphasort __mpls_alphasort
#define scandir __mpls_scandir

#endif /* __MPLS_SDK_SUPPORT_NEW_SCANDIR__ && ... */

#endif /* __DARWIN_C_LEVEL >= 200809L */

#endif /* _MACPORTS_DIRENT_H_ */
