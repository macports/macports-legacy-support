
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

#ifndef _MACPORTS_SYSSTAT_H_
#define _MACPORTS_SYSSTAT_H_



/* Include the primary system stat.h */
#include_next <sys/stat.h>

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MP_LEGACY_SUPPORT_ATCALLS__
#ifdef __cplusplus
extern "C" {
#endif
  extern int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
  extern int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags);
  extern int mkdirat(int dirfd, const char *pathname, mode_t mode);
#ifdef __cplusplus
}
#endif
#endif /* __MP_LEGACY_SUPPORT_ATCALLS__ */



#endif /* _MACPORTS_SYSSTAT_H_ */
