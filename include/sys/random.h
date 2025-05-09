/*
 * Copyright (c) 2021
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

#ifndef _MACPORTS_SYS_RANDOM_H_
#define _MACPORTS_SYS_RANDOM_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/*
 * In the systems that need this, sys/random.h requires the u_int typedef,
 * which not only depends on sys/types.h, but also may be blocked by POSIX
 * settings.  In that case, we provide our own typedef if appropriate, or
 * else we avoid the include_next that won't work.
 *
 * The OS versions with getentropy() don't appear to filter it based on
 * POSIX settings, so we do the same here.
 *
 * In the later OSes that don't need our addition, sys/random.h is completely
 * different and doesn't require u_int, and thus the include_next is
 * unconditional.
 */

#if __MPLS_SDK_SUPPORT_GETENTROPY__

#if !defined(_POSIX_C_SOURCE) \
    || (defined(_DARWIN_C_SOURCE) && __MPLS_SDK_MAJOR >= 1050)

/* Add the missing typedef (redundancy shouldn't hurt ). */
typedef	unsigned int u_int;

/* Include the primary system sys/random.h */
#include_next <sys/random.h>

#endif /* (!_POSIX_C_SOURCE || (_DARWIN_C_SOURCE && >10.4)) */

__MP__BEGIN_DECLS
extern int getentropy(void *buf, size_t buflen);
__MP__END_DECLS

#else /* !__MPLS_SDK_SUPPORT_GETENTROPY__ */

/* Include the primary system sys/random.h */
#include_next <sys/random.h>

#endif /* !__MPLS_SDK_SUPPORT_GETENTROPY__ */

#endif /* _MACPORTS_SYS_RANDOM_H_ */
