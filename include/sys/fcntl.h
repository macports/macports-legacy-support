/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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

#ifndef _MACPORTS_SYS_FCNTL_H_
#define _MACPORTS_SYS_FCNTL_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system sys/fcntl.h */
#include_next <sys/fcntl.h>

/* Provide missing O_CLOEXEC definition for <10.7. */
#ifndef O_CLOEXEC
#define O_CLOEXEC 0x1000000
#endif

#if __DARWIN_C_LEVEL >= 200809L

/* "at" functions */
#if __MPLS_SDK_SUPPORT_ATCALLS__

#define AT_FDCWD		-2  /*Descriptor value for the current working directory */
#define AT_EACCESS		0x0010	/* Use effective ids in access check */
#define AT_SYMLINK_NOFOLLOW	0x0020	/* Act on the symlink itself not the target */
#define AT_SYMLINK_FOLLOW	0x0040	/* Act on target of symlink */
#define AT_REMOVEDIR		0x0080	/* Path refers to directory */

__MP__BEGIN_DECLS

/*
 * Note: Avoids 'dirfd' as a parameter name due to 'shadowing' complaints from
 * some compilers.
 */

extern int openat(int __dirfd, const char *pathname, int flags, ...);

__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_ATCALLS__ */

#endif /* __DARWIN_C_LEVEL >= 200809L */

#endif /* _MACPORTS_SYS_FCNTL_H_ */
