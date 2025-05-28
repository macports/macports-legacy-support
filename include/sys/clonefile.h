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

#ifndef _MACPORTS_SYS_CLONEFILE_H_
#define _MACPORTS_SYS_CLONEFILE_H_

/*
 * This is a wrapper/replacement header for sys/clonefile.h, handling its
 * absence in the <10.12 SDKs.  In those cases, we provide substitute
 * definitions; otherwise we just pass through the SDK header as is.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_SUPPORT_CLONEFILE__

/* Options for clonefile calls */
#define CLONE_NOFOLLOW   0x0001     /* Don't follow symbolic links */

#include <stdint.h>

__MP__BEGIN_DECLS

int clonefileat(int, const char *, int, const char *, uint32_t);
int fclonefileat(int, int, const char *, uint32_t);
int clonefile(const char *, const char *, uint32_t);

__MP__END_DECLS

#else  /* !__MPLS_SDK_SUPPORT_CLONEFILE__ */

#include_next <sys/clonefile.h>

#endif  /* !__MPLS_SDK_SUPPORT_CLONEFILE__ */

#endif /* _MACPORTS_SYS_CLONEFILE_H_ */
