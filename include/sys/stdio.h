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

#ifndef _MACPORTS_SYS_STDIO_H_
#define _MACPORTS_SYS_STDIO_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system sys/stdio.h (10.10+ only) */
#if __MPLS_SDK_MAJOR >= 101000
#include_next <sys/stdio.h>
#endif

#if __MPLS_SDK_SUPPORT_ATCALLS__

/* renameat */
__MP__BEGIN_DECLS
extern int renameat(int olddirfd, const char *oldpath,
                    int newdirfd, const char *newpath);
__MP__END_DECLS

#endif /* __MPLS_SDK_SUPPORT_ATCALLS__ */

#endif /* _MACPORTS_SYS_STDIO_H_ */
