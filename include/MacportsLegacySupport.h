/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

/*
 * Not needed directly -- #include <AvailabilityMacros.h>,
 * but see <_macports_extras/sdkversion.h>
 *
 * NOTE: Not including AvailabilityMacros.h (directly or indirectly)
 * makes it safe to include this header before sdkversion.h.
 */

/* C++ extern definitions */
#if defined(__cplusplus)
#define	__MP__BEGIN_DECLS extern "C" {
#define	__MP__END_DECLS	  }
#else
#define	__MP__BEGIN_DECLS
#define	__MP__END_DECLS
#endif

/* foundational defs, used later */

/* True for Apple-only i386 build */
#if defined(__i386) && __APPLE__
#define __MPLS_APPLE_I386__            1
#else
#define __MPLS_APPLE_I386__            0
#endif

/*
 * More concise and more comprehensive target OS definition, to simplify
 * many conditionals.
 *
 * Compilers provide __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__.
 * When -mmacosx-version-min is supplied, this macro is defined appropriately.
 * When it isn't supplied, Xcode 3+ compilers define it based on the host OS.
 * Prior compilers don't define it at all in this case.
 *
 * In the undefined case, Apple's AvailabilityMacros.h define it as either
 * 10.4 or 10.1, depending on the hardware architecture.  Since we don't
 * support anything earlier than 10.4, this condition is unnecessary.
 *
 * In the non-Apple case (__APPLE__ undefined), we define our macro as a large
 * number, to disable all "version < X" cases.  This is equivalent to ANDing
 * the condition with __APPLE__.
 *
 * We also allow the definition to be overridden for special circumstances,
 * though this isn't normally necessary.
 */
#ifndef __MPLS_TARGET_OSVER
#if __APPLE__
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define __MPLS_TARGET_OSVER __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define __MPLS_TARGET_OSVER 1040
#endif
#else /* !__APPLE__ */
#define __MPLS_TARGET_OSVER 999999
#endif /* !__APPLE__ */
#endif /* __MPLS_TARGET_OSVER undef */

/*
 * Defines for when legacy support is required for various functions:
 *
 * In the general case, each feature flag should really be two feature
 * flags, one referring to the SDK version and one referring to the target
 * OS version.  These will both refer to the same OS version, but applied
 * in different ways.
 *
 * If a given feature is implemented entirely in the headers, then only
 * the SDK-based flag needs to exist.  But it's highly unlikely that a feature
 * would be implemented solely in the library without header support, so it's
 * highly unlikely that the library flag would exist without the SDK flag.
 *
 * Occasionally, a header-only macro-based feature may require only an
 * #ifndef as a condition, in which case no feature flag is necessary.
 *
 * With rare exception, the tests shouldn't directly reference any of these
 * flags at all, since the intent is that a test should behave the same
 * regardless of whether the feature is provided by the OS or by this
 * package.
 *
 * In the new (not yet fully applied) naming scheme, the two flags for
 * a given feature are named:
 *    __MPLS_SDK_<feature>
 *    __MPLS_LIB_<feature>
 *
 * The first flag is based on an appropriate __MPLS_PRE_<xxx>_SDK, which
 * needs to exist in sdkversion.h, and files using it need to include
 * that header (as well as this one).
 *
 * The second flag is typically defined as a comparison on __MPLS_TARGET_OSVER,
 * though in some cases the condition may be more complicated.
 */

/* fsgetpath */
#define __MP_LEGACY_SUPPORT_FSGETPATH__       (__MPLS_TARGET_OSVER < 101300)

/* **setattrlistat */
#define __MP_LEGACY_SUPPORT_SETATTRLISTAT__   (__MPLS_TARGET_OSVER < 101300)

/* ** utimensat, futimens, UTIME_NOW, UTIME_OMIT */
#define __MP_LEGACY_SUPPORT_UTIMENSAT__       (__MPLS_TARGET_OSVER < 101300)

/* clock_gettime */
#define __MP_LEGACY_SUPPORT_GETTIME__         (__MPLS_TARGET_OSVER < 101200)

/* timespec_get */
#define __MP_LEGACY_SUPPORT_TIMESPEC_GET__    (__MPLS_TARGET_OSVER < 101500)

/* **at calls */
#define __MP_LEGACY_SUPPORT_ATCALLS__         (__MPLS_TARGET_OSVER < 101000)

/* fdopendir */
#define __MP_LEGACY_SUPPORT_FDOPENDIR__       (__MPLS_TARGET_OSVER < 101000)

/* <net/if.h> include <sys/socket.h> */
#define __MPLS_SDK_NETIF_SOCKET_FIX__         __MPLS_PRE_10_9_SDK

/* CMSG_DATA definition in <sys/socket.h> */
#define __MPLS_SDK_CMSG_DATA_FIX__            __MPLS_PRE_10_6_SDK

/* stpncpy */
#define __MPLS_SDK_SUPPORT_STPNCPY__          __MPLS_PRE_10_7_SDK
#define __MPLS_LIB_SUPPORT_STPNCPY__          (__MPLS_TARGET_OSVER < 1070)

/* strnlen */
#define __MP_LEGACY_SUPPORT_STRNLEN__         (__MPLS_TARGET_OSVER < 1070)

/* strndup */
#define __MP_LEGACY_SUPPORT_STRNDUP__         (__MPLS_TARGET_OSVER < 1070)

/* dprintf */
#define __MP_LEGACY_SUPPORT_DPRINTF__         (__MPLS_TARGET_OSVER < 1070)

/* getline */
#define __MP_LEGACY_SUPPORT_GETLINE__         (__MPLS_TARGET_OSVER < 1070)

/* memmem */
#define __MP_LEGACY_SUPPORT_MEMMEM__          (__MPLS_TARGET_OSVER < 1070)

/* wcsdup */
#define __MP_LEGACY_SUPPORT_WCSDUP__          (__MPLS_TARGET_OSVER < 1070)

/* wcsnlen */
#define __MP_LEGACY_SUPPORT_WCSNLEN__         (__MPLS_TARGET_OSVER < 1070)

/* wcpcpy, wcpncpy */
#define __MP_LEGACY_SUPPORT_WCPCPY__          (__MPLS_TARGET_OSVER < 1070)

/* wcsncasecmp_l, wcscasecmp_l, wcsncasecmp, wcscasecmp */
#define __MP_LEGACY_SUPPORT_WCSCASECMP__      (__MPLS_TARGET_OSVER < 1070)

/* llround */
#define __MP_LEGACY_SUPPORT_LLROUND__         (__MPLS_TARGET_OSVER < 1070)

/* arc4random */
#define __MPLS_SDK_SUPPORT_ARC4RANDOM__       __MPLS_PRE_10_7_SDK
#define __MPLS_LIB_SUPPORT_ARC4RANDOM__       (__MPLS_TARGET_OSVER < 1070)

/* getentropy */
#define __MP_LEGACY_SUPPORT_GETENTROPY__      (__MPLS_TARGET_OSVER < 101200)

/* posix_memalign does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__  (__MPLS_TARGET_OSVER < 1060)

/* AI_NUMERICSERV does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_AI_NUMERICSERV__  (__MPLS_TARGET_OSVER < 1060)

/*  realpath() on < 1060 does not support modern NULL buffer usage */
#define __MP_LEGACY_SUPPORT_REALPATH_WRAP__   (__MPLS_TARGET_OSVER < 1060)

/* setattrlistat */
#define __MP_LEGACY_SUPPORT_FSETATTRLIST__    (__MPLS_TARGET_OSVER < 1060)

/* localtime_r, gmtime_r, etc only declared on Tiger when _ANSI_SOURCE and _POSIX_C_SOURCE are undefined */
#define __MPLS_SDK_SUPPORT_TIME_THREAD_SAFE_FUNCTIONS__  __MPLS_PRE_10_5_SDK

/* lsmod does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LSMOD__           (__MPLS_TARGET_OSVER < 1050)

/* lutimes does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LUTIMES__         (__MPLS_TARGET_OSVER < 1050)

/* sys/aio.h header needs adjustment to match newer SDKs */
#define __MPLS_SDK_SYS_AIO_TIGER_FIX__        __MPLS_PRE_10_5_SDK

/*  sysconf() is missing some functions on some systems, and may misbehave on i386 */
#define __MP_LEGACY_SUPPORT_SYSCONF_WRAP__    (__MPLS_TARGET_OSVER < 101100 \
                                               || __MPLS_APPLE_I386__)

/* pthread_rwlock_initializer is not defined until 10.5 */
#define __MPLS_SDK_SUPPORT_PTHREAD_RWLOCK__   __MPLS_PRE_10_5_SDK

/* STAILQ_FOREACH is not defined on Tiger*/
#define __MP_LEGACY_SUPPORT_STAILQ_FOREACH__  (__MPLS_TARGET_OSVER < 1050)

/* c++11 <cmath> PPC 10.[45] and Intel 10.[4-6], GNU g++ 4.6 through 8. */
#if (__MPLS_TARGET_OSVER < 1070 \
               && defined(__GNUC__) && (__GNUC__ <= 8)                 \
               && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))))
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 1
#else
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 0
#endif

/* cossin */
#define __MP_LEGACY_SUPPORT_COSSIN__  (__MPLS_TARGET_OSVER < 1090)

/* ffsl */
#define __MP_LEGACY_SUPPORT_FFSL__    (__MPLS_TARGET_OSVER < 1050)
/* ffsll */
#define __MP_LEGACY_SUPPORT_FFSLL__   (__MPLS_TARGET_OSVER < 1090)

/* fls */
#define __MP_LEGACY_SUPPORT_FLS__     (__MPLS_TARGET_OSVER < 1050)
/* flsl */
#define __MP_LEGACY_SUPPORT_FLSL__    (__MPLS_TARGET_OSVER < 1050)
/* flsll */
#define __MP_LEGACY_SUPPORT_FLSLL__   (__MPLS_TARGET_OSVER < 1090)

/* open_memstream */
#define __MP_LEGACY_SUPPORT_OPEN_MEMSTREAM__  (__MPLS_TARGET_OSVER < 101300)

/* fmemopen */
#define __MP_LEGACY_SUPPORT_FMEMOPEN__  (__MPLS_TARGET_OSVER < 101300)

/* pthread_setname_np */
#define __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__  (__MPLS_TARGET_OSVER < 1060)

/* Compound macros, bundling functionality needed by multiple single features. */
#define __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__  (__MP_LEGACY_SUPPORT_ATCALLS__ || __MP_LEGACY_SUPPORT_SETATTRLISTAT__)

#define __MP_LEGACY_SUPPORT_NEED_BEST_FCHDIR__    (__MP_LEGACY_SUPPORT_FDOPENDIR__ || __MP_LEGACY_SUPPORT_ATCALLS__ || __MP_LEGACY_SUPPORT_SETATTRLISTAT__)

/* UUIDs - for now, just add missing typedef statements */
#define __MPLS_SDK_SUPPORT_UUID__  __MPLS_PRE_10_6_SDK

/* for now, just forward call to CFPropertyListCreateWithStream */
#define __MPLS_SDK_SUPPORT_CoreFoundation__  __MPLS_PRE_10_6_SDK

/* copyfile and its associated functions have gained functionality over the years */
#define __MP_LEGACY_SUPPORT_COPYFILE_WRAP__ (__MPLS_TARGET_OSVER < 1060)

/* _tlv_atexit and __cxa_thread_atexit */
#define __MP_LEGACY_SUPPORT_ATEXIT_WRAP__  (__MPLS_TARGET_OSVER < 1070)

/* os_unfair_lock structure and its associated functions */
#define __MP_LEGACY_SUPPORT_OS_UNFAIR_LOCK__  (__MPLS_TARGET_OSVER < 101200)

/* library symbol ___bzero */
#define __MP_LEGACY_SUPPORT_SYMBOL____bzero__  (__MPLS_TARGET_OSVER < 1060)

/* library symbol _dirfd */
#define __MP_LEGACY_SUPPORT_SYMBOL__dirfd__  (__MPLS_TARGET_OSVER < 1080)

/* fix bug in pthread_get_stacksize_np */
/* see https://github.com/rust-lang/rust/issues/43347 */
#define __MP_LEGACY_SUPPORT_PTHREAD_GET_STACKSIZE_NP_FIX__ (__MPLS_TARGET_OSVER == 101000 \
                                                            || __MPLS_TARGET_OSVER == 1090 \
                                                            || __MPLS_TARGET_OSVER <  1060 )

#endif /* _MACPORTS_LEGACYSUPPORTDEFS_H_ */
