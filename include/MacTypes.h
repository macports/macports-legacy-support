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
 * Beginning with the macOS 15 SDK, this header expects the __has_include()
 * operator to be available.  If it isn't, then this file fails to include
 * ConditionalMacros.h, causing errors.  Since that's the only use of this
 * operator in this file, we just temporarily define it as an always-true
 * dummy, and then undo that after the include_next.
 *
 * Since sys/cdefs.h may supply an inappropriate fallback for __has_include(),
 * we need to use the flag set up by our version to determine the proper
 * behavior.
 *
 * To avoid accidentally and possibly inappropriately applying this workaround
 * to a future SDK, we limit it with a both-ways SDK version check.  This
 * should be updated as needed when new SDKs are added.
 */

/* Determine the true __has_include() status (if not already done) */
#include <sys/cdefs.h>

/* See if __has_include() is missing or lying */
#if __MPLS_HAS_INCLUDE_STATUS <= 0

/* Determine the SDK version */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR >= 150000 && __MPLS_SDK_MAJOR < 160000
  #undef __has_include
  #define __has_include(x) 1
  #define __MPLS_HAS_INCLUDE_CHANGED
#endif

#endif /* __has_include undef */

/* Include the primary system MacTypes.h */
#include_next <MacTypes.h>

#ifdef __MPLS_HAS_INCLUDE_CHANGED
  #undef __has_include
  #if __MPLS_HAS_INCLUDE_STATUS == 0
    #define __has_include(x) 0
  #endif
  #undef __MPLS_HAS_INCLUDE_CHANGED
#endif
