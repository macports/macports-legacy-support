/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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

/* realpath wrapper */
#if __MPLS_LIB_SUPPORT_REALPATH_WRAP__

/* Keep stdlib from defining any version of realpath */
#define realpath __MPLS_hide_realpath

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>  /* For NULL */
#include <stdlib.h>

#include <sys/syslimits.h>  /* For PATH_MAX */

/* Now undo our macro kludge */
#undef realpath

/*
 * This provides a wrapper for realpath() in order to make the 10.6+
 * optional buffer allocation available in <10.6.  At present, it does not
 * make the optional 10.5+ semantic improvements available in 10.4.
 *
 * It provides wrappers for all versions of realpath(), even when targeting
 * 10.4, since the client may be built with a later SDK.  It attempts to
 * pass through to the same version as the one invoked (thereby maintaining
 * the semantics), but falls back to the basic version if that fails (normally
 * only on 10.4).  If that fails, it bails with an abort(), since that should
 * be impossible with an uncorrupted system library.
 *
 * The address of the OS realpath() is cached, to avoid repeating the dlsym()
 * lookup on every call.  In a fallback case, the address of the basic
 * version is cached as if it were the address of the intended version, since
 * the intended version isn't going to magically appear later.
 */

/* Function type for realpath() */
typedef char *rp_func_t(const char * __restrict, char * __restrict);

/* Macro defining all versions */
/* Note that the UNIX2003 version never exists in 64-bit builds. */

#if !__MPLS_64BIT

#define RP_ALL \
  RP_ENT(,,basic) \
  RP_ENT($,UNIX2003,posix) \
  RP_ENT($,DARWIN_EXTSN,darwin)

#else /* 64-bit */

#define RP_ALL \
  RP_ENT(,,basic) \
  RP_ENT($,DARWIN_EXTSN,darwin)

#endif /* 64-bit */

/* Table of indices of versions */
#define RP_ENT(d,x,t) rp_##t,
typedef enum {
  RP_ALL
} rp_idx_t;
#undef RP_ENT

/* Table of names */
#define RP_STR(x) #x
#define RP_ENT(d,x,t) RP_STR(realpath##d##x),
static const char *rp_name[] = {
  RP_ALL
};
#undef RP_ENT

/* Table of cached addresses */
#define RP_ENT(d,x,t) NULL,
static rp_func_t *rp_adr[] = {
  RP_ALL
};
#undef RP_ENT

/* Internal realpath(), with version as a parameter */
static char *
realpath_internal(const char * __restrict file_name,
                  char * __restrict resolved_name,
                  rp_idx_t version)
{
  rp_func_t *os_realpath;
  char *buf, *result;
  int saved_errno;

  /* Locate proper OS realpath(), with fallback if needed */
  if (!(os_realpath = rp_adr[version])) {
    os_realpath = rp_adr[version] = dlsym(RTLD_NEXT, rp_name[version]);
    if (!os_realpath && version != rp_basic) {
      os_realpath = rp_adr[version] = dlsym(RTLD_NEXT, rp_name[rp_basic]);
    }
    if (!os_realpath) abort();
  }

  /* Just pass through the call if a buffer was supplied */
  if (resolved_name) return (*os_realpath)(file_name, resolved_name);

  /* Otherwise allocate a buffer and invoke it with that */
  if (!(buf = malloc(PATH_MAX))) return NULL;
  if ((result = (*os_realpath)(file_name, buf))) return result;

  /* On failure, free the allocated buffer */
  /* Although free() shouldn't touch errno, we preserve it just in case */
  saved_errno = errno;
  free(buf);
  errno = saved_errno;
  return NULL;
}

/* Now the various public realpath() versions */

#define RP_ENT(d,x,t) \
char * \
realpath##d##x(const char * __restrict file_name, \
               char * __restrict resolved_name) \
{ \
  return realpath_internal(file_name, resolved_name, rp_##t); \
}
RP_ALL
#undef RP_ENT

/*
 * Compatibility function to avoid the need to rebuild existing binaries
 * built with the old wrapper-macro implementation (between Jan-2019 and
 * Apr-2022).  We have no way to determine which realpath version was
 * used in such a build (since that information was destroyed by the wrapper
 * macro), so we assume the usual default for the SDK matching the target
 * OS for *this* build.  That's the best we can do under the circumstances.
 */

#if __MPLS_TARGET_OSVER < 1050
#define RP_DEFAULT rp_basic
#else
#define RP_DEFAULT rp_darwin
#endif

char *
macports_legacy_realpath(const char * __restrict file_name,
                         char * __restrict resolved_name)
{
  return realpath_internal(file_name, resolved_name, RP_DEFAULT);
}

#endif /*__MPLS_LIB_SUPPORT_REALPATH_WRAP__*/
