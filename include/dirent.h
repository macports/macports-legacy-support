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
#if __MPLS_SDK_SUPPORT_ATCALLS__

__MP__BEGIN_DECLS

#ifndef __DARWIN_ALIAS_I
extern DIR *fdopendir(int fd) __DARWIN_ALIAS(fdopendir);
#else
extern DIR *fdopendir(int fd) __DARWIN_ALIAS_I(fdopendir);
#endif

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_ATCALLS__ */

/* New signature for scandir and alphasort (optionally) */

/* These functions are non-POSIX, so avoid broken refs. */
#if !defined(_POSIX_C_SOURCE) \
    || (defined(_DARWIN_C_SOURCE) && __MPLS_SDK_MAJOR >= 1050)

#if __MPLS_SDK_SUPPORT_NEW_SCANDIR__

/*
 * Here we provide compatibility wrappers so that scandir and alphasort
 * references based on the new function signatures can be used with the
 * old versions of the functions.  To avoid breaking code expecting the
 * normal behavior, the wrapper macros are only introduced conditionally,
 * based on _MACPORTS_LEGACY_COMPATIBLE_SCANDIR being nonzero.  The wrapper
 * *functions* are defined unconditionally, since, by themselves, they don't
 * introduce a conflict.
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
 *
 *
 * It's possible that some clients will have issues with the wrapper macros,
 * since they're defined as simple macros which could match other uses of
 * the same identifiers (e.g. stucture elements).  This could be avoided by
 * using function macros, but those wouldn't work in contexts where taking
 * pointers to them is required.  Instead, we provide the following for
 * alternative approaches:
 *
 * 1) The wrapper *functions* are defined for all SDK versions, so that they
 * may be used without version tests.  With newer SDKs, they are simple
 * transparent wrappers with no casts.
 *
 * 2) The macro _MACPORTS_LEGACY_OLD_SCANDIR is provided as the condition
 * where the compatibility issue exists, to allow conditional workarounds
 * without the need for (direct) awareness of SDK versions.  This macro
 * is defined for all SDK versions, but only nonzero when the signature
 * issue exists.
 *
 * NOTE:  We need to avoid using 'dirname' as a parameter name, since some
 * compilers may complain about "shadowing" the 'dirname' function.
 */

__MP__BEGIN_DECLS

static __inline__ int
__mpls_alphasort(const struct dirent **d1, const struct dirent **d2)
{
  /*
   * The casts here are currently unnecessary, but could be needed in
   * the future if compilers get pickier.
   */
  return alphasort((const void *) d1, (const void *) d2);
}

static __inline__ int
__mpls_scandir(const char *dirnam, struct dirent ***namelist,
               int (*selector)(const struct dirent *),
               int (*compar)(const struct dirent **, const struct dirent **))
{
  return scandir(dirnam, namelist,
                 (int (*)(struct dirent *)) selector,
                 (int (*)(const void *, const void *)) compar);
}

__MP__END_DECLS

/* Define the wrapper macros if requested */
#if defined(_MACPORTS_LEGACY_COMPATIBLE_SCANDIR) \
    && _MACPORTS_LEGACY_COMPATIBLE_SCANDIR
#define alphasort __mpls_alphasort
#define scandir __mpls_scandir
#endif

#else /* !__MPLS_SDK_SUPPORT_NEW_SCANDIR__  */

/* Dummy wrapper functions without unnecessary casts */

static __inline__ int
__mpls_alphasort(const struct dirent **d1, const struct dirent **d2)
{
  return alphasort(d1, d2);
}

static __inline__ int
__mpls_scandir(const char *dirnam, struct dirent ***namelist,
               int (*selector)(const struct dirent *),
               int (*compar)(const struct dirent **, const struct dirent **))
{
  return scandir(dirnam, namelist, selector, compar);
}

#endif /* !__MPLS_SDK_SUPPORT_NEW_SCANDIR__  */

#endif /* (!_POSIX_C_SOURCE || (_DARWIN_C_SOURCE && >10.4)) */

#endif /* __DARWIN_C_LEVEL >= 200809L */

/* Provide a testable condition for the scandir signature issue. */
#define _MACPORTS_LEGACY_OLD_SCANDIR __MPLS_SDK_SUPPORT_NEW_SCANDIR__

#endif /* _MACPORTS_DIRENT_H_ */
