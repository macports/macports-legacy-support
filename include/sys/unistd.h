
/*
 * Copyright (c) 2010 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2018 Ken Cunningham <kencu@macports.org>
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

#ifndef _MACPORTS_SYSUNISTD_H_
#define _MACPORTS_SYSUNISTD_H_



/* Include the primary system unistd.h */
#include_next <sys/unistd.h>

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MP_LEGACY_SUPPORT_ATCALLS__

/* typedefs to prevent loading many headers */
typedef long ssize_t;	
typedef unsigned long size_t;
typedef __uint32_t	uid_t;
typedef __uint32_t	gid_t;

#ifdef __cplusplus
extern "C" {
#endif
  extern int getattrlistat(int dirfd, const char *pathname, struct attrlist *a,
                                 void *buf, size_t size, unsigned long flags);

  extern ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

  extern int faccessat(int dirfd, const char *pathname, int mode, int flags);
  extern int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
  extern int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
  extern int symlinkat(const char *oldpath, int newdirfd, const char *newpath);
  extern int unlinkat(int dirfd, const char *pathname, int flags);
  
#ifdef __cplusplus
}
#endif
#endif /* __MP_LEGACY_SUPPORT_ATCALLS__ */



#endif /* _MACPORTS_SYSUNISTD_H_ */
