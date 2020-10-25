
/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2018 Ken Cunningham <kencu@macports.org>
 * Copyright (c) 2019 Michael Dickens <michaelld@macports.org>
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

#ifndef _MACPORTS_LEGACYSUPPORTDEFS_H_
#define _MACPORTS_LEGACYSUPPORTDEFS_H_

/* Not needed -- #include "AvailabilityMacros.h" */
#include "MacportsLegacyWrappers/wrapper_macros.h"

/* C++ extern definitions */
#if defined(__cplusplus)
#define	__MP__BEGIN_DECLS extern "C" {
#define	__MP__END_DECLS	  }
#else
#define	__MP__BEGIN_DECLS
#define	__MP__END_DECLS
#endif

/* defines for when legacy support is required for various functions */

/* fsgetpath */
#define __MP_LEGACY_SUPPORT_FSGETPATH__       (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* clock_gettime */
#define __MP_LEGACY_SUPPORT_GETTIME__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200)

/* **at calls */
#define __MP_LEGACY_SUPPORT_ATCALLS__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101000)

/* fdopendir */
#define __MP_LEGACY_SUPPORT_FDOPENDIR__       (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101000)


/* this header is automatically included by <net/if.h> on systems 10.9 and up.
   It is therefore expected to be included by most current software. */
/* <net/if.h> include <sys/socket.h> */
#define __MP_LEGACY_SUPPORT_NETIF_SOCKET_FIX__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* strnlen */
#define __MP_LEGACY_SUPPORT_STRNLEN__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* strndup */
#define __MP_LEGACY_SUPPORT_STRNDUP__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* getline */
#define __MP_LEGACY_SUPPORT_GETLINE__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* memmem */
#define __MP_LEGACY_SUPPORT_MEMMEM__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsdup */
#define __MP_LEGACY_SUPPORT_WCSDUP__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsnlen */
#define __MP_LEGACY_SUPPORT_WCSNLEN__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcpcpy, wcpncpy */
#define __MP_LEGACY_SUPPORT_WCPCPY__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsncasecmp_l, wcscasecmp_l, wcsncasecmp, wcscasecmp */
#define __MP_LEGACY_SUPPORT_WCSCASECMP__      (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* llround */
#define __MP_LEGACY_SUPPORT_LLROUND__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* arc4random */
#define __MP_LEGACY_SUPPORT_ARC4RANDOM__      (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* posix_memalign does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* AI_NUMERICSERV does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_AI_NUMERICSERV__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/*  realpath() on < 1060 does not support modern NULL buffer usage */
#define __MP_LEGACY_SUPPORT_REALPATH_WRAP__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/*  realpath() wrap has bail-out macros in case we want to disable only function wrapping */
#define __ENABLE_MP_LEGACY_SUPPORT_REALPATH_WRAP__  (!__DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__  && \
						     !__DISABLE_MP_LEGACY_SUPPORT_REALPATH_WRAP__      && \
						     __MP_LEGACY_SUPPORT_REALPATH_WRAP__)

/* lsmod does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LSMOD__           (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* lutimes does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LUTIMES__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/*  sysconf() is missing some functions on some systems */
#define __MP_LEGACY_SUPPORT_SYSCONF_WRAP__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101100)

/*  sysconf() wrap has bail-out macros in case we want to disable only function wrapping */
#define __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__  (!__DISABLE_MP_LEGACY_SUPPORT_FUNCTION_WRAPPING__  && \
						    !__DISABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__       && \
						    __MP_LEGACY_SUPPORT_SYSCONF_WRAP__)

/* pthread_rwlock_initializer is not defined on Tiger */
#define __MP_LEGACY_SUPPORT_PTHREAD_RWLOCK__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* STAILQ_FOREACH is not defined on Tiger*/
#define __MP_LEGACY_SUPPORT_STAILQ_FOREACH__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* c++11 <cmath> PPC 10.[45] and Intel 10.[4-6], GNU g++ 4.6 through 8. */
#if (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070 \
               && defined(__GNUC__) && (__GNUC__ <= 8)                 \
               && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))))
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 1
#else
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 0
#endif

/* cossin */
#define __MP_LEGACY_SUPPORT_COSSIN__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* ffsl */
#define __MP_LEGACY_SUPPORT_FFSL__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* ffsll */
#define __MP_LEGACY_SUPPORT_FFSLL__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* fls */
#define __MP_LEGACY_SUPPORT_FLS__     (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* flsl */
#define __MP_LEGACY_SUPPORT_FLSL__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* flsll */
#define __MP_LEGACY_SUPPORT_FLSLL__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)


#endif /* _MACPORTS_LEGACYSUPPORTDEFS_H_ */
