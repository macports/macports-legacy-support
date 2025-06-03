/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * NOTICE: This file was modified in May 2024, and again in June 2025, to allow
 * for use as a supporting file for MacPorts legacy support library. This
 * notice is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 *
 * The code is almost verbatim from Apple except for:
 *
 * The correction of the return type.
 *
 * The removal of the 'restrict' qualifiers for compatibility with
 * pre-C99 compilers.
 *
 * The addition of the missing 'const' qualifier.
 *
 * The _FORTIFY_SOURCE definition here in lieu of providing it as a
 * compiler command-line flag (as the Apple build procedure does).
 *
 * Making the reference to __chk_fail optional, to handle building
 * with a "mismatched" SDK.  This uses dlsym() instead of weak linking.
 *
 * The inclusion of OS version conditionals.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

/* Note that the support for this mechanism is absent prior to 10.5 */
#if __MPLS_LIB_SUPPORT_STPNCPY__ && __MPLS_TARGET_OSVER >= 1050

/* Ensure that we don't create an infinitely recursive check function */
#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

char *
__stpncpy_chk (char *dest, const char *src, size_t len, size_t dstlen)
{
  void (*chk_fail_p) (void) __attribute__((__noreturn__));

  if (__builtin_expect (dstlen < len, 0)) {
    if ((chk_fail_p = dlsym(RTLD_NEXT, "__chk_fail"))) (*chk_fail_p)();
    abort();
  }

  return stpncpy (dest, src, len);
}

#endif /* __MPLS_LIB_SUPPORT_STPNCPY__  && >= 10.5 */
