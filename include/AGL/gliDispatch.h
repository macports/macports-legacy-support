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
 * In SDKs > 10.6 this Apple header does not include glext.h.
 * Including it causes redefinition errors that are hard to
 * overcome in ports, eg mesa, so we block the loading of
 * glext.h here on older systems for consistent behaviour with newer systems.
 *
 * Note: this header has no guard macro as it may be called
 * multiple times and should have the same effect each time.
 *
 * Also note:  This header doesn't exist in 10.7+ AGL, so there's no need for
 * a version check.  Any attempt to use this wrapper with a 10.7+ SDK will
 * result in a failure of the include_next.
 */

#ifdef __glext_h_
#  define __MPLS_SAVED_GLEXT_SET
#else
#  define __glext_h_
#endif

/* Include the primary system AGL/gliDispatch.h */
#include_next <AGL/gliDispatch.h>

#ifdef __MPLS_SAVED_GLEXT_SET
#  undef __MPLS_SAVED_GLEXT_SET
#else
#  undef __glext_h_
#endif
