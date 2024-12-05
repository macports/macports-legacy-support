/*
 * originally from atcalls.c
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
  * NOTICE: This chunk of header was refactored from the above Apple-supplied
  * file, atcalls.c. The relevant part of that header was moved here.
  * for use as a supporting file for MacPorts legacy support library. This notice
  * is included in support of clause 2.2 (b) of the Apple Public License,
  * Version 2.0.
  */

#ifndef _MACPORTS_COMMON_PRIV_H_
#define _MACPORTS_COMMON_PRIV_H_

#include <fcntl.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/fcntl.h>

#define PROTECT_ERRNO(what)  ({ int __err = (errno); what; errno = __err; })
#define ERR_ON(code, what)   if (what) { errno = (code); return -1; }

#ifndef SYS___pthread_fchdir
# define SYS___pthread_fchdir 349
#endif

int best_fchdir(int dirfd);

#define _ATCALL(fd, p, onerr, what)                             \
    ({  typeof(what) __result;                                  \
        int oldCWD = -1;                                        \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            oldCWD = open(".", O_RDONLY);                       \
            if (best_fchdir(-1) < 0 && oldCWD != -1) {          \
                close(oldCWD); oldCWD = -1;                     \
            }                                                   \
            if (best_fchdir(fd) < 0) {                          \
                PROTECT_ERRNO(best_fchdir(oldCWD));             \
                if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD)); \
                return onerr;                                   \
            }                                                   \
        }                                                       \
        __result = (what);                                      \
        if (fd != AT_FDCWD && p[0] != '/') {                    \
            PROTECT_ERRNO(best_fchdir(oldCWD));                 \
            if (oldCWD != -1) PROTECT_ERRNO(close(oldCWD));     \
        }                                                       \
        __result;                                               \
    })

#define ATCALL(fd, p, what)  _ATCALL(fd, p, -1, what)

#endif /* _MACPORTS_COMMON_PRIV_H_ */
