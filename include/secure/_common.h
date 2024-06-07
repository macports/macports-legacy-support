/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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
 * Support for security wrappers is absent in 10.4.  It's also not enabled
 * in the 10.4 SDK, but building for 10.4 with a later SDK and explicit
 * enabling results in build failures.  So we undefine the enable on 10.4.
 *
 * Note that this is more aggressive than the setup in _types.h in the 10.6+
 * SDK, which only *defaults* it off on 10.4.  But that results in build
 * failures when _FORTIFY_SOURCE is set nonzero explicitly.
 */
#if !defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050
#undef _FORTIFY_SOURCE
#endif

/*
 * Include the primary system secure/_common.h
 *
 * That header is nonexistent in the 10.4 SDK, but so are refernces to it,
 * so if we're building with a 10.4 SDK we won't be here.
 */
#include_next <secure/_common.h>
