/*
 * Copyright (c) 2019 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2019 Ken Cunningham <kencu@macports.org>
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

#ifndef _MACPORTS_UNISTD_H_
#define _MACPORTS_UNISTD_H_

/* MP support header */
#include "MacportsLegacySupport.h"

#if __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* redefine the original sysconf */
#undef sysconf
#define sysconf(a) sysconf_orig(a)

#endif /*__ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__*/

#include_next <unistd.h>

#if __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* and now define sysconf as our new wrapped function */
#undef sysconf
#include "MacportsLegacyWrappers/sysconf_support.h"

__MP__BEGIN_DECLS
extern long sysconf(int) __MP_LEGACY_WRAPPER_ALIAS(sysconf);
__MP__END_DECLS

#endif /* __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__ */

#endif /* _MACPORTS_UNISTD_H_ */
