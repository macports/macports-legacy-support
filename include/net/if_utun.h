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

/*
 * This is a wrapper header for net/if_utun.h, to handle its absence in
 * the <10.6 SDKs.  In those cases, we provide a substitute; otherwise we
 * just pass through the SDK header.
 *
 * This makes no representation as to whether the features referenced by
 * the definitions in this header actually work in <10.6.
 *
 * We don't bother with a guard macro, since all we do here is include
 * other headers which have their own guard macros, and we don't define
 * anything here.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if defined(__MPLS_SDK_MAJOR) && __MPLS_SDK_MAJOR < 1060
#include <_macports_extras/tiger_leopard/net/if_utun.h>
#else
#include_next <net/if_utun.h>
#endif
