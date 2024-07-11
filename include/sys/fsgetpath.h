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

#ifndef _MACPORTS_SYS_FSGETPATH_H_
#define _MACPORTS_SYS_FSGETPATH_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system sys/fsgetpath.h (10.13+ only) */
#if __MPLS_SDK_MAJOR >= 101300
#include_next <sys/fsgetpath.h>
#endif

#if __MPLS_SDK_SUPPORT_FSGETPATH__

__MP__BEGIN_DECLS
extern ssize_t fsgetpath(char * __restrict buf, size_t bufsize, fsid_t* fsid, uint64_t objid);
__MP__END_DECLS

#if __MPLS_TARGET_OSVER < 1060
#error "No implementation of fsgetpath is presently available for MacOSX prior to 10.6"
#endif

#endif /* __MPLS_SDK_SUPPORT_FSGETPATH__ */

#endif /* _MACPORTS_SYS_FSGETPATH_H_ */
