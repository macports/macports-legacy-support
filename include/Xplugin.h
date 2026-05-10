/*
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

#ifndef _MACPORTS_XPLUGIN_H_
#define _MACPORTS_XPLUGIN_H_

/*
 * This header is nominally included in the 10.4 SDK, but missing from the
 * version of the latter provided with Xcode 2.5.  It is present in the
 * 10.4 SDKs provided with Xcode 3.1+, as well as all 10.5+ SDKs.
 *
 * This wrapper header provides a copy of the 10.4 version when using a
 * 10.4 SDK, and otherwise just passes through the SDK version.  It's
 * neither convenient nor necessary to determine whether the header is
 * already present in the 10.4 case, so we provide our copy unconditionally.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1050
#include <_macports_extras/tiger_only/Xplugin.h>
#else
#include_next <Xplugin.h>
#endif

#endif /* _MACPORTS_XPLUGIN_H_ */
