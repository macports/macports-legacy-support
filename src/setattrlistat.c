/*
 * setattrlistat.c
 *
 * Copyright (c) 2013-2015 Apple Inc. All rights reserved.
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
  * NOTICE: This file was modified in September 2020 to allow
  * for use as a supporting file for MacPorts legacy support library. This notice
  * is included in support of clause 2.2 (b) of the Apple Public License,
  * Version 2.0.
  */


/* MP support header */
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_SETATTRLISTAT__

#include "common-priv.h"

#include <sys/attr.h>
#include <assert.h>
#include <stdint.h>


int setattrlistat(int dirfd, const char *pathname, void *a,
                  void *buf, size_t size, uint32_t flags)
{
    int cont = 1,
        ret = 0;

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1080
    /*
     * Older systems don't correctly check if no attributes are to be set, which usually
     * means a buffer size of zero and return an error since they malloc a block of
     * memory with size zero, leading to ENOMEM.
     *
     * Emulate the fix from 10.8 for those.
     */
    const struct attrlist *al = a;
    if (al->commonattr == 0 &&
        (al->volattr & ~ATTR_VOL_INFO) == 0 &&
        al->dirattr == 0 &&
        al->fileattr == 0 &&
        al->forkattr == 0) {
        cont = 0;
    }
#endif

    if (cont) {
        ret = ATCALL(dirfd, pathname, setattrlist(pathname, a, buf, size, flags));
    }

    return ret;
}

#endif  /* __MP_LEGACY_SUPPORT_SETATTRLISTAT__ */
