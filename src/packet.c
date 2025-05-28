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

/*
 * Handle the suffixed variants of recvmsg(), even if no fixes being applied.
 *
 * Define a macro listing all variants supported by the OS/arch.
 * 10.4 lacks the NOCANCEL variant.
 * 64-bit builds lack the UNIX2003 variant.
 *
 * The macro defined here only includes the non-basic variants, so that
 * the inclusion of the basic case can be optional.
 *
 * Note that additional aliases appeared in 10.7+, but since this code
 * doesn't currently apply there, we ignore that.
 *
 * Although 10.4 doesn't have NOCANCEL, builds with later SDKs may
 * reference it, so we need to provide it anyway.  The runtime lookup
 * falls back to the basic version if it's missing.
 */

#if !__MPLS_64BIT

#define MOST_VARIANTS \
  VARIANT_ENT(posix,$UNIX2003) \
  VARIANT_ENT(nocancel,$NOCANCEL$UNIX2003)

#else /* __MPLS_64BIT */

#define MOST_VARIANTS \
  VARIANT_ENT(nocancel,$NOCANCEL)

#endif /* __MPLS_64BIT */

#if __MPLS_LIB_CMSG_ROSETTA_FIX__ || __MPLS_LIB_CMSG_FORMAT_FIX__

/*
 * There are at least three known issues related to packet timestamps in
 * OS versions prior to 10.7.  In order of discovery, they are:
 *
 *   1) The 10.5 sys/socket.h has a bad definition of CMSG_DATA which
 * inappropriately pads the header length to 16 bytes in 64-bit builds,
 * resulting in an incorrect payload address.  This is fixed in our wrapper
 * header.
 *
 *   2) In 64-bit builds running on a 32-bit <10.6 kernel, the struct timeval
 * supplied by the kernel is based on a 32-bit time_t, while userspace expects
 * a version based on a 64-bit time_t.  This is fixed here, by reformatting
 * the relevant payloads.
 *
 *   3) Although Rosetta correctly byte-swaps CMSG headers, it fails
 * to byte-swap the payloads, resulting in garbled timestamps.  This is
 * fixed here, by applying the missing byte swaps.
 *
 * The fix for #2 is applied before the fix for #3, so that the latter need
 * only concern itself with the expected format.  But in practice, the two
 * issues never occur simultaneously, since #2 only applies to 64-bit builds,
 * and Rosetta doesn't support ppc64.  In fact, at present, the two fixes
 * aren't simultaneously present in the code, since the Rosetta fix is only
 * present in ppc builds, and the format fix is only present in pre-10.6
 * 64-bit builds.  But the code design doesn't rely on that.
 *
 * Issue #1 applies to all CMSG types, and is fixed for all types.  It is
 * not known at this time whether issues #2 and/or #3 apply to other CMSG
 * types.  If so, the code could be extended appropriately.
 */

/*
 * The non-noncancelable variants are the default.  This results in
 * generating the "unadorned" names, which we augment with explicit suffixes
 * where needed.  Similarly, we use the non-POSIX form in 32-bit builds.
 */

#if !__MPLS_64BIT
#define _NONSTD_SOURCE
#endif

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "compiler.h"
#include "endian.h"

#define CMSG_DATALEN(cmsg) ((uint8_t *) (cmsg) + (cmsg)->cmsg_len \
	                    - (uint8_t *) CMSG_DATA(cmsg))

#define FORMAT_FIX   __MPLS_LIB_CMSG_FORMAT_FIX__
#define ROSETTA_FIX  __MPLS_LIB_CMSG_ROSETTA_FIX__

#define ALL_VARIANTS \
  VARIANT_ENT(basic,) \
  MOST_VARIANTS

#define VARIANT_ENT(name,sfx) fv_##name,
typedef enum fv_type {
  ALL_VARIANTS
} fv_type_t;
#undef VARIANT_ENT

#define VARIANT_ENT(name,sfx) "recvmsg" #sfx,
static const char * const fv_names[] = {
  ALL_VARIANTS
};
#undef VARIANT_ENT

typedef __typeof__(recvmsg) recvmsg_fn_t;

#define VARIANT_ENT(name,sfx) NULL,
static recvmsg_fn_t *fv_adrs[] = {
  ALL_VARIANTS
};
#undef VARIANT_ENT

/* Function to get address of the OS function, with fallback for 10.4 */
static recvmsg_fn_t *
sys_recvmsg(fv_type_t fvtype)
{
  /* Return cached value if available */
  if (MPLS_FASTPATH(fv_adrs[fvtype])) return fv_adrs[fvtype];

  /* Or cache and return address of desired variant if available */
  if ((fv_adrs[fvtype] = dlsym(RTLD_NEXT, fv_names[fvtype]))) {
    return fv_adrs[fvtype];
  }

  /* If not, try for basic version (10.4) */
  if ((fv_adrs[fvtype] = dlsym(RTLD_NEXT, fv_names[fv_basic]))) {
    return fv_adrs[fvtype];
  }

  /* Something's badly wrong if we can't find the function at all */
  abort();
}

#if FORMAT_FIX

/*
 * Handling for mismatched timestamp formats, adapted from similar code
 * developed for ntpsec's ntpd.
 *
 * This covers payload sizes of 8, 12, and 16 bytes, though only the 8-byte
 * case is expected to occur in practice in macOS (32-bit kernel with 64-bit
 * userspace in <10.6).
 *
 * For more details, see:
 * https://gitlab.com/NTPsec/ntpsec/-/commit/7c238744b4eb1990ed1135b93fc6fbfc6c576a05
 */

#if defined(__ppc__) || defined(__ppc64__)
#define IS_LITTLE_ENDIAN 0
#else
#define IS_LITTLE_ENDIAN 1
#endif

#define MAX_TV_USEC 1000000

static void
fetch_cmsg_timeval(struct cmsghdr *cmsghdr, struct timeval *tvp)
{
	struct timeval_3232 {
		uint32_t	tv_sec;  /* Unsigned to get past 2038 */
		int32_t		tv_usec;
	} *tv3232p;
	struct timeval_6432 {
		int64_t		tv_sec;
		int32_t		tv_usec;
	} *tv6432p;
	#define SIZEOF_PACKED_TIMEVAL6432 (sizeof(int64_t) + sizeof(int32_t))
	struct timeval_6464 {
		int64_t		tv_sec;
		uint32_t	tv_usec[2];  /* Unsigned for compares */
	} *tv6464p;
	int datalen = CMSG_DATALEN(cmsghdr);

	if (datalen == sizeof(struct timeval)) {
		*tvp = *((struct timeval *) CMSG_DATA(cmsghdr));
		return;
	}

	switch (datalen) {

	case sizeof(struct timeval_3232):
		tv3232p = (struct timeval_3232 *) CMSG_DATA(cmsghdr);
		tvp->tv_sec = tv3232p->tv_sec;
		tvp->tv_usec = tv3232p->tv_usec;
		return;

	case SIZEOF_PACKED_TIMEVAL6432:
		tv6432p = (struct timeval_6432 *) CMSG_DATA(cmsghdr);
		tvp->tv_sec = tv6432p->tv_sec;
		tvp->tv_usec = tv6432p->tv_usec;
		return;

	case sizeof(struct timeval_6464):
		tv6464p = (struct timeval_6464 *) CMSG_DATA(cmsghdr);
		tvp->tv_sec = tv6464p->tv_sec;
		if (IS_LITTLE_ENDIAN) {
			tvp->tv_usec = tv6464p->tv_usec[0];
			return;
		} else if (tv6464p->tv_usec[0] == 0) {
			if (tv6464p->tv_usec[1] < MAX_TV_USEC) {
				tvp->tv_usec = tv6464p->tv_usec[1];
			} else {
				tvp->tv_usec = tv6464p->tv_usec[0];
			}
			return;
		} else if (tv6464p->tv_usec[0] < MAX_TV_USEC) {
			tvp->tv_usec = tv6464p->tv_usec[0];
			return;
		}
		/* FALLTHRU to default (invalid timestamp) */

	default:
		memset(tvp, 0, sizeof(*tvp));
	}
}

/* Check message lengths, to see if format adjustments are needed */
static int
check_cmsg_lengths(struct msghdr *msghdr, socklen_t *new_controllen)
{
	struct cmsghdr *cmsghdr;
	int lenadj;
	int needadj = 0;

  cmsghdr = CMSG_FIRSTHDR(msghdr);
  *new_controllen = 0;

  while (cmsghdr) {
    if (cmsghdr->cmsg_level == SOL_SOCKET) {

      lenadj = 0;

      switch (cmsghdr->cmsg_type) {

      case SCM_TIMESTAMP:
        lenadj = sizeof(struct timeval) - CMSG_DATALEN(cmsghdr);
        break;
      }

      *new_controllen += cmsghdr->cmsg_len + lenadj;
      if (lenadj) needadj = 1;
    }
    cmsghdr = CMSG_NXTHDR(msghdr, cmsghdr);
  }
  return needadj;
}

/* Reformat any messages that need it */
static void
fix_cmsg_formats(struct msghdr *msghdr, socklen_t new_controllen)
{
	struct cmsghdr *cmsghdr, *newhdr;
	struct timeval *tvp;
	uint8_t *newcmsg, cbuf[1024];

  /* If our local buffer won't hold the new contents, punt */
  if (new_controllen > sizeof(cbuf)) return;
  newcmsg = &cbuf[0];

  cmsghdr = CMSG_FIRSTHDR(msghdr);

  while (cmsghdr) {
    if (cmsghdr->cmsg_level != SOL_SOCKET) {
      memcpy(newcmsg, cmsghdr, cmsghdr->cmsg_len);
      newcmsg += cmsghdr->cmsg_len;
    } else {

      switch (cmsghdr->cmsg_type) {

      case SCM_TIMESTAMP:
        newhdr = (struct cmsghdr *) newcmsg;
        *newhdr = *cmsghdr;
        newhdr->cmsg_len = sizeof(*cmsghdr) + sizeof(*tvp);
        tvp = (struct timeval *) (newcmsg + sizeof(*cmsghdr));
        fetch_cmsg_timeval(cmsghdr, tvp);
        newcmsg += newhdr->cmsg_len;
        break;

      default:
        memcpy(newcmsg, cmsghdr, cmsghdr->cmsg_len);
        newcmsg += cmsghdr->cmsg_len;
        break;
      }
    }
    cmsghdr = CMSG_NXTHDR(msghdr, cmsghdr);
  }
  /* Punt if new total length isn't as expected */
  if (newcmsg != &cbuf[new_controllen]) return;

  /* Else replace the cmsg stream with the new one */
  memcpy(msghdr->msg_control, cbuf, new_controllen);
  msghdr->msg_controllen = new_controllen;
}

#else /* !FORMAT_FIX */

static int
check_cmsg_lengths(struct msghdr *msghdr, socklen_t *new_controllen)
{
  (void) msghdr; (void) new_controllen;
  return 0;
}

static void
fix_cmsg_formats(struct msghdr *msghdr, socklen_t new_controllen)
{
  (void) msghdr; (void) new_controllen;
}

#endif /* !FORMAT_FIX */

#if ROSETTA_FIX

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

#else /* !ROSETTA_FIX */

static int check_rosetta(void) { return -1; }
static void fix_cmsg_endianness(struct msghdr *msghdr) { (void) msghdr; }

#endif /* !ROSETTA_FIX */

/* Common internal function for all variants */
static ssize_t
recvmsg_internal(int socket, struct msghdr *message, int flags,
                 fv_type_t fvtype)
{
  static int is_rosetta = 0;
  socklen_t init_controllen, new_controllen;
  ssize_t ret;

  /* Determine Rosettaness, if not already known */
  if (MPLS_SLOWPATH(!is_rosetta)) is_rosetta = check_rosetta();

  /* Just pass through if Rosetta-only and not Rosetta */
  if (!FORMAT_FIX && is_rosetta < 0) {
    return (*sys_recvmsg(fvtype))(socket, message, flags);
  }

  /* Need to intercept return (first capturing initial controllen) */
  init_controllen = message->msg_control ? message->msg_controllen : 0;
  ret = (*sys_recvmsg(fvtype))(socket, message, flags);

  /* If error or no CMSG data, just return */
  if (ret < 0 || !message->msg_controllen) return ret;

  /* Otherwise, fix the data as needed */

  /* First see if any formats need adjusting (by checking lengths) */
  if (check_cmsg_lengths(message, &new_controllen)) {
    /* Reformat as needed, if the result will still fit */
    if (new_controllen <= init_controllen) {
      fix_cmsg_formats(message, new_controllen);
    }
  }

  /* Now, if Rosetta, do any needed byte-swapping */
  if (is_rosetta > 0) fix_cmsg_endianness(message);

  return ret;
}

#define VARIANT_ENT(name,sfx) \
ssize_t recvmsg##sfx(int socket, struct msghdr *message, int flags) \
  { return recvmsg_internal(socket, message, flags, fv_##name); }
ALL_VARIANTS
#undef VARIANT_ENT

/* Dummy wrapper for avoiding fixes */
static ssize_t
recvmsg_dummy(int socket, struct msghdr *message, int flags, fv_type_t fvtype)
{
  return (*sys_recvmsg(fvtype))(socket, message, flags);
}

#define VARIANT_ENT(name,sfx) \
ssize_t __mpls_standard_recvmsg##sfx( \
    int socket, struct msghdr *message, int flags) \
  { return recvmsg_dummy(socket, message, flags, fv_##name); }
ALL_VARIANTS
#undef VARIANT_ENT

#elif __MPLS_TARGET_OSVER < 1050 /* 10.4 with no fixes */

/* If we're not applying the fix on 10.4, provide dummy wrappers */

#include <sys/types.h>
struct msghdr;
ssize_t recvmsg(int socket, struct msghdr *message, int flags);

#define VARIANT_ENT(name,sfx) \
ssize_t recvmsg##sfx(int socket, struct msghdr *message, int flags) \
  { return recvmsg(socket, message, flags); }
MOST_VARIANTS
#undef VARIANT_ENT

#endif /* 10.4 with no fixes */
