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

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_FIX_64BIT_BOOTTIME__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysctl.h>
#include <sys/time.h>

#include "util.h"

/*
 * Under OS <10.6, the returned struct timeval for boottime is always based on
 * the kernel's 32-bit time_t, even with a 64-bit userspace that expects
 * the 64-bit time_t version of struct timeval.  This error happens to be
 * somewhat benign on little-endian machines, but results in complete garbage
 * on big-endian machines (ppc64).
 *
 * To fix this, we detect the misbehavior by observing the incorrect
 * length, and reformat the data appropriately.
 */

typedef struct timeval timeval_t;

typedef struct tv32_s {
  uint32_t tv_sec;  /* Unsigned to get past 2038 */
  int32_t  tv_usec;
} tv32_t;

/* See if we got the wrong boottime format, and fix it if so */
static void
fix_boottime(timeval_t *oldp, size_t *oldlenp, size_t origlen)
{
  tv32_t tv32;

  /* If we wanted a timeval and got a tv32 ... */
  if (oldp && origlen >= sizeof(*oldp) 
      && *oldlenp == sizeof(tv32_t) && sizeof(tv32_t) < sizeof(*oldp)) {
    tv32 = *((tv32_t *) oldp);
    oldp->tv_sec = tv32.tv_sec;
    oldp->tv_usec = tv32.tv_usec;
    *oldlenp = sizeof(*oldp);
  }
}

int
sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
       void *newp, size_t newlen)
{
  int ret;
  size_t origlen;
  GET_OS_FUNC(sysctl)

  /* Capture originally specified length */
  origlen = oldlenp ? *oldlenp : 0;

  /* Do the call; return error on failure */
  ret = (*os_sysctl)(name, namelen, oldp, oldlenp, newp, newlen);
  if (ret) return ret;

  /* If we just obtained boottime, possibly correct it */
  if (namelen >=2 && name[0] == CTL_KERN && name[1] == KERN_BOOTTIME) {
    fix_boottime(oldp, oldlenp, origlen);
  }

  return 0;
}

/*
 * The same boottime issue also applies to sysctlbyname().  We apply
 * the same fix to that call.  This does not correct the absence of
 * this item on 10.4.
 */

int
sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
       void *newp, size_t newlen)
{
  int ret;
  size_t origlen;
  GET_OS_FUNC(sysctlbyname)

  /* Capture originally specified length */
  origlen = oldlenp ? *oldlenp : 0;

  /* Do the call; return error on failure */
  ret = (*os_sysctlbyname)(name, oldp, oldlenp, newp, newlen);
  if (ret) return ret;

  /* If we just obtained boottime, possibly correct it */
  if (!strcmp(name, "kern.boottime")) {
    fix_boottime(oldp, oldlenp, origlen);
  }

  return 0;
}

#endif /* __MPLS_LIB_FIX_64BIT_BOOTTIME__ */
