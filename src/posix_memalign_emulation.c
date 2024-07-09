/*
 * Copyright (c) 1999, 2006-2008 Apple Inc. All rights reserved.
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
  Inspired in part by malloc (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea and released to the public domain, as explained at
  http://creativecommons.org/publicdomain/zero/1.0/ Send questions,
  comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
   Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
         Check before installing!
*/

 /*
  * NOTICE: This file was modified in December 2018 to allow
  * use as a supporting file for MacPorts legacy support library. This notice
  * is included in support of clause 2.2 (b) of the Apple Public License,
  * Version 2.0.
  */

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_POSIX_MEMALIGN__

#include <stdlib.h>
#include <errno.h>

int posix_memalign(void** pp, size_t alignment, size_t bytes) {

  /* if alignment is 0 or not a power of 2 return bad value */
  if (alignment < sizeof( void *) ||            // excludes 0 == alignment
      0 != (alignment & (alignment - 1))) {     // relies on sizeof(void *) being a power of two.
    return EINVAL;
  }

  void* mem = 0;

  if (alignment <= 16) {

    /* MacOSX always returns memory aligned on
     * a 16 byte alignment
     */
    mem = malloc(bytes);

  } else {

   /* if the caller wants a larger alignment than 16
    * we give them a page-aligned allotment. This is not as efficient
    * as an optimized aligned memory implementation, but much
    * simpler, effective, and requires no changes to the rest of the
    * underlying memory management system.
    */
    mem = valloc(bytes);
  }
  if (mem == 0)
    return ENOMEM;
  else {
    *pp = mem;
    return 0;
  }
}

#endif /* __MPLS_LIB_SUPPORT_POSIX_MEMALIGN__ */
