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

#if __MPLS_LIB_CMSG_ROSETTA_FIX__

/*
 * There are at least three known issues related to packet timestamps in
 * OS versions prior to 10.7.  In order of discovery, they are:
 *
 *   1) The 10.5 sys/socket.h has a bad definition of CMSG_DATA which
 * inappropriately pads the header length to 16 bytes in 64-bit builds,
 * resulting in an incorrect payload address.  This is fixed in our wrapper
 * header.
 *
 *   2) In 64-bit builds running on a 32-bit 10.4-10.5 kernel,
 * the struct timeval supplied by the kernel is based on a 32-bit time_t,
 * while userspace expects a version based on a 64-bit time_t.  This is
 * not currently fixed here, but ntpsec has its own workaround for it.
 * A future version might fix this, which would involve recopying the entire
 * CMSG stream to adjust any lengths that need it.
 *
 *   3) Although Rosetta correctly byte-swaps the CMSG header, it fails
 * to byte-swap the payload, resulting in garbled timestamps.  That is
 * the issue addressed here.  The fix is only applied when the payload
 * length is as expected, but since issue #2 only applies to 64-bit builds
 * and Rosetta doesn't support ppc64, that's not a problem.
 *
 * Issue #1 applies to all CMSG types, and is fixed for all types.  It is
 * not known at this time whether issues #2 and/or #3 apply to other CMSG
 * types.  If so, the code could be extended appropriately.
 */

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "endian.h"

#define CMSG_DATALEN(cmsg) ((uint8_t *) (cmsg) + (cmsg)->cmsg_len \
	                    - (uint8_t *) CMSG_DATA(cmsg))

/* sysctl to check whether we're running natively (non-ppc only) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* Test whether it's Rosetta */
/* -1 no, 1 yes */
static int
check_rosetta(void)
{
  int native;
  size_t native_sz = sizeof(native);

  if (sysctlbyname(SYSCTL_NATIVE, &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    return -1;
  }
  return native ? -1 : 1;
}

/* Fix endianness of CMSG payloads */
static void
fix_cmsg_endianness(struct msghdr *msghdr)
{
	struct cmsghdr *cmsghdr;
	struct timeval *tvp;

  cmsghdr = CMSG_FIRSTHDR(msghdr);

  while (cmsghdr) {
    if (cmsghdr->cmsg_level == SOL_SOCKET) {

      switch (cmsghdr->cmsg_type) {

      case SCM_TIMESTAMP:
        if (CMSG_DATALEN(cmsghdr) != sizeof(*tvp)) break;
        tvp = (struct timeval *) CMSG_DATA(cmsghdr);
        BYTE_SWAP_INPLACE(tvp->tv_sec);
        BYTE_SWAP_INPLACE(tvp->tv_usec);
        break;
      }
    }
    cmsghdr = CMSG_NXTHDR(msghdr, cmsghdr);
  }
}

ssize_t
recvmsg(int socket, struct msghdr *message, int flags)
{
  static __typeof__(recvmsg) *sys_recvmsg = NULL;
  static int is_rosetta = 0;
  ssize_t ret;

  if (!sys_recvmsg) {
    if (!(sys_recvmsg = dlsym(RTLD_NEXT, "recvmsg"))){
      /* Something's badly wrong if we can't find the function */
      abort();
    }
  }

  /* Determine Rosettaness, if not already known */
  if (!is_rosetta) is_rosetta = check_rosetta();

  /* Just pass through if not Rosetta */
  if (is_rosetta < 0) return (*sys_recvmsg)(socket, message, flags);

  /* Running under Rosetta - need to intercept return */
  ret = (*sys_recvmsg)(socket, message, flags);

  /* If error or no CMSG data, just return */
  if (ret < 0 || !message->msg_controllen) return ret;

  /* Otherwise, fix the data */
  fix_cmsg_endianness(message);

  return ret;
}

#endif /* __MPLS_LIB_CMSG_ROSETTA_FIX__ */
