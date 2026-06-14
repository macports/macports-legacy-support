/*
 * Copyright (c) 2026
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
 * The 10.4 version of this header has five typedefs which differ from the
 * 10.5+ versions.  To eliminate this inconsistency, we substitute a
 * modified version of the header in the 10.4 case.  This can't be done
 * as a wrapper header, since typedefs can't be overridden.  In the 10.5+
 * cases, we simply pass through the standard version.
 */

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1050
#include <_macports_extras/tiger_only/OpenGL/gl.h>
#else
#include_next <OpenGL/gl.h>
#endif
