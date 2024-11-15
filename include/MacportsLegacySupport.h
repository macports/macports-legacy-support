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
 * NOTE: Not including AvailabilityMacros.h (directly or indirectly) here
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

/* True for 64-bit build */
#if defined(__LP64__) && __LP64__
#define __MPLS_64BIT 1
#else
#define __MPLS_64BIT 0
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
 * the SDK-based flag needs to exist.  In a few cases, a feature is
 * implemented solely in the library without added header support,
 * in which case the library flag exists without the SDK flag.
 *
 * Occasionally, a header-only macro-based feature may require only an
 * #ifndef as a condition, in which case no feature flag is necessary.
 *
 * With rare exception, the tests shouldn't directly reference any of these
 * flags at all, since the intent is that a test should behave the same
 * regardless of whether the feature is provided by the OS or by this
 * package.
 *
 * In the new naming scheme, the two flags for a given feature are named:
 *    __MPLS_SDK_<feature>
 *    __MPLS_LIB_<feature>
 *
 * The first flag is defined as a comparison on __MPLS_SDK_MAJOR,
 * and files using it need to include sdkversion.h (as well as this one).
 *
 * The second flag is typically defined as a comparison on __MPLS_TARGET_OSVER,
 * though in some cases the condition may be more complicated.
 *
 * NOTE: At present, no attempt is made to correct the availability attributes
 * of definitions obtained from the SDK.  When using an SDK matched to the
 * target OS version, this is a non-issue, since any feature provided by
 * that OS version would be shown as available in that OS version, and any
 * feature not provided by the target OS would be declared/defined in our
 * own headers with no availability attributes.  But if a build uses a later
 * SDK for an OS version that provides the feature, then it would usually
 * show as unavailable for any OS version where our own support is needed.
 * This is likely to produce availability warnings when such warnings are
 * enabled.  If any fix for this is possible, it would be likely to involve
 * considerable effort to implement; hence the problem is ignored for now.
 * That means that either building with "later" SDKs should be avoided, or
 * both enabling availability warnings and treating warnings as errors
 * should be avoided.
 *
 * NOTE: When adding new features, be sure to respect __DARWIN_C_LEVEL in
 * the relevant header(s) if applicable, and add appropriate test cases
 * to manual_tests/darwin_c.c if so.
 */

/* fsgetpath */
#define __MPLS_SDK_SUPPORT_FSGETPATH__        (__MPLS_SDK_MAJOR < 101300)
#define __MPLS_LIB_SUPPORT_FSGETPATH__        (__MPLS_TARGET_OSVER < 101300)

/* setattrlistat */
#define __MPLS_SDK_SUPPORT_SETATTRLISTAT__    (__MPLS_SDK_MAJOR < 101300)
#define __MPLS_LIB_SUPPORT_SETATTRLISTAT__    (__MPLS_TARGET_OSVER < 101300)

/* ** utimensat, futimens, UTIME_NOW, UTIME_OMIT */
#define __MPLS_SDK_SUPPORT_UTIMENSAT__        (__MPLS_SDK_MAJOR < 101300)
#define __MPLS_LIB_SUPPORT_UTIMENSAT__        (__MPLS_TARGET_OSVER < 101300)

/* clock_gettime */
#define __MPLS_SDK_SUPPORT_GETTIME__          (__MPLS_SDK_MAJOR < 101200)
#define __MPLS_LIB_SUPPORT_GETTIME__          (__MPLS_TARGET_OSVER < 101200)

/* timespec_get */
#define __MPLS_SDK_SUPPORT_TIMESPEC_GET__     (__MPLS_SDK_MAJOR < 101500)
#define __MPLS_LIB_SUPPORT_TIMESPEC_GET__     (__MPLS_TARGET_OSVER < 101500)

/* "at" calls */
#define __MPLS_SDK_SUPPORT_ATCALLS__          (__MPLS_SDK_MAJOR < 101000)
#define __MPLS_LIB_SUPPORT_ATCALLS__          (__MPLS_TARGET_OSVER < 101000)

/* fdopendir */
#define __MPLS_SDK_SUPPORT_FDOPENDIR__        (__MPLS_SDK_MAJOR < 101000)
#define __MPLS_LIB_SUPPORT_FDOPENDIR__        (__MPLS_TARGET_OSVER < 101000)

/* new signature for scandir and alphasort */
#define __MPLS_SDK_SUPPORT_NEW_SCANDIR__      (__MPLS_SDK_MAJOR < 1080)

/* <net/if.h> include <sys/socket.h> */
#define __MPLS_SDK_NETIF_SOCKET_FIX__         (__MPLS_SDK_MAJOR < 1090)

/* CMSG_DATA definition in <sys/socket.h> */
#define __MPLS_SDK_CMSG_DATA_FIX__            (__MPLS_SDK_MAJOR < 1060)

/* stpncpy */
#define __MPLS_SDK_SUPPORT_STPNCPY__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_STPNCPY__          (__MPLS_TARGET_OSVER < 1070)

/* strnlen */
#define __MPLS_SDK_SUPPORT_STRNLEN__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_STRNLEN__          (__MPLS_TARGET_OSVER < 1070)

/* strndup */
#define __MPLS_SDK_SUPPORT_STRNDUP__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_STRNDUP__          (__MPLS_TARGET_OSVER < 1070)

/* dprintf, vdprintf */
#define __MPLS_SDK_SUPPORT_DPRINTF__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_DPRINTF__          (__MPLS_TARGET_OSVER < 1070)

/* getline */
#define __MPLS_SDK_SUPPORT_GETLINE__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_GETLINE__          (__MPLS_TARGET_OSVER < 1070)

/* memmem */
#define __MPLS_SDK_SUPPORT_MEMMEM__           (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_MEMMEM__           (__MPLS_TARGET_OSVER < 1070)

/* wcsdup */
#define __MPLS_SDK_SUPPORT_WCSDUP__           (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_WCSDUP__           (__MPLS_TARGET_OSVER < 1070)

/* wcsnlen */
#define __MPLS_SDK_SUPPORT_WCSNLEN__          (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_WCSNLEN__          (__MPLS_TARGET_OSVER < 1070)

/* wcpcpy, wcpncpy */
#define __MPLS_SDK_SUPPORT_WCPCPY__           (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_WCPCPY__           (__MPLS_TARGET_OSVER < 1070)

/* wcsncasecmp_l, wcscasecmp_l, wcsncasecmp, wcscasecmp */
#define __MPLS_SDK_SUPPORT_WCSCASECMP__       (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_WCSCASECMP__       (__MPLS_TARGET_OSVER < 1070)

/* llround */
#define __MPLS_SDK_SUPPORT_LLROUND__          (__MPLS_SDK_MAJOR < 1070)

/* arc4random */
#define __MPLS_SDK_SUPPORT_ARC4RANDOM__       (__MPLS_SDK_MAJOR < 1070)
#define __MPLS_LIB_SUPPORT_ARC4RANDOM__       (__MPLS_TARGET_OSVER < 1070)

/* getentropy */
#define __MPLS_SDK_SUPPORT_GETENTROPY__       (__MPLS_SDK_MAJOR < 101200)
#define __MPLS_LIB_SUPPORT_GETENTROPY__       (__MPLS_TARGET_OSVER < 101200)

/* posix_memalign does not exist on < 10.6 */
#define __MPLS_SDK_SUPPORT_POSIX_MEMALIGN__   (__MPLS_SDK_MAJOR < 1060)
#define __MPLS_LIB_SUPPORT_POSIX_MEMALIGN__   (__MPLS_TARGET_OSVER < 1060)

/* AI_NUMERICSERV does not exist on < 10.6 */
/* The addition uses an #ifndef, so no feature flag is necessary */

/*  realpath() on < 10.6 does not support modern NULL buffer usage */
#define __MPLS_LIB_SUPPORT_REALPATH_ALLOC__   (__MPLS_TARGET_OSVER < 1060)

/* Also, 10.6 non-POSIX realpath() (32-bit only) with a nonexistent path and
 * a NULL buffer returns an unsafe pointer to an internal buffer. */
#define __MPLS_LIB_SUPPORT_REALPATH_NONEX_FIX__ (__MPLS_TARGET_OSVER >= 1060 \
                                                 && __MPLS_TARGET_OSVER < 1070 \
                                                 && !__MPLS_64BIT)

/* fsetattrlistat, fgetattrlistat */
#define __MPLS_SDK_SUPPORT_FSETATTRLIST__     (__MPLS_SDK_MAJOR < 1060)
#define __MPLS_LIB_SUPPORT_FSETATTRLIST__     (__MPLS_TARGET_OSVER < 1060)

/* localtime_r, gmtime_r, etc only declared on Tiger when _ANSI_SOURCE and _POSIX_C_SOURCE are undefined */
#define __MPLS_SDK_SUPPORT_TIME_THREAD_SAFE_FUNCTIONS__  (__MPLS_SDK_MAJOR < 1050)

/* lchmod does not exist on Tiger */
#define __MPLS_SDK_SUPPORT_LCHMOD__           (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_LCHMOD__           (__MPLS_TARGET_OSVER < 1050)

/* lutimes does not exist on Tiger */
#define __MPLS_SDK_SUPPORT_LUTIMES__          (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_LUTIMES__          (__MPLS_TARGET_OSVER < 1050)

/* sys/aio.h header needs adjustment to match newer SDKs */
#define __MPLS_SDK_SYS_AIO_TIGER_FIX__        (__MPLS_SDK_MAJOR < 1050)

/*  sysconf() is missing some functions on some systems, and may misbehave on i386 */
#define __MPLS_SDK_SUPPORT_SYSCONF_NPROCESSORS__  (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_SYSCONF_NPROCESSORS__  (__MPLS_TARGET_OSVER < 1050)

#define __MPLS_SDK_SUPPORT_SYSCONF_PHYS_PAGES__   (__MPLS_SDK_MAJOR < 101100)
#define __MPLS_LIB_SUPPORT_SYSCONF_PHYS_PAGES__   (__MPLS_TARGET_OSVER < 101100 \
                                                   || !__MPLS_64BIT)

#define __MPLS_LIB_SUPPORT_SYSCONF_WRAP__ (__MPLS_LIB_SUPPORT_SYSCONF_NPROCESSORS__ \
                                           || __MPLS_LIB_SUPPORT_SYSCONF_PHYS_PAGES__)

/* PTHREAD_RWLOCK_INITIALIZER is not defined until 10.5 */
/* The addition uses an #ifndef, so no feature flag is necessary */

/* Macros missing from some earlier versions of sys/queue.h */
/* All additions use #ifndef, so no feature flags are necessary. */

/* Missing until 10.5 */
/* SLIST_HEAD_INITIALIZER, STAILQ_FOREACH */

/* Missing until 10.7 */
/* SLIST_REMOVE_AFTER */

/* c++11 <cmath> PPC 10.[45] and Intel 10.[4-6], GNU g++ 4.6 through 8. */
#if (__MPLS_TARGET_OSVER < 1070 \
               && defined(__GNUC__) && (__GNUC__ <= 8)                 \
               && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))))
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 1
#else
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 0
#endif

/* cossin */
#define __MPLS_SDK_SUPPORT_COSSIN__   (__MPLS_SDK_MAJOR < 1090)
#define __MPLS_LIB_SUPPORT_COSSIN__   (__MPLS_TARGET_OSVER < 1090)

/* ffsl */
#define __MPLS_SDK_SUPPORT_FFSL__     (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_FFSL__     (__MPLS_TARGET_OSVER < 1050)
/* ffsll */
#define __MPLS_SDK_SUPPORT_FFSLL__    (__MPLS_SDK_MAJOR < 1090)
#define __MPLS_LIB_SUPPORT_FFSLL__    (__MPLS_TARGET_OSVER < 1090)

/* fls */
#define __MPLS_SDK_SUPPORT_FLS__      (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_FLS__      (__MPLS_TARGET_OSVER < 1050)
/* flsl */
#define __MPLS_SDK_SUPPORT_FLSL__     (__MPLS_SDK_MAJOR < 1050)
#define __MPLS_LIB_SUPPORT_FLSL__     (__MPLS_TARGET_OSVER < 1050)
/* flsll */
#define __MPLS_SDK_SUPPORT_FLSLL__    (__MPLS_SDK_MAJOR < 1090)
#define __MPLS_LIB_SUPPORT_FLSLL__    (__MPLS_TARGET_OSVER < 1090)

/* open_memstream */
#define __MPLS_SDK_SUPPORT_OPEN_MEMSTREAM__   (__MPLS_SDK_MAJOR < 101300)
#define __MPLS_LIB_SUPPORT_OPEN_MEMSTREAM__   (__MPLS_TARGET_OSVER < 101300)

/* fmemopen */
#define __MPLS_SDK_SUPPORT_FMEMOPEN__   (__MPLS_SDK_MAJOR < 101300)
#define __MPLS_LIB_SUPPORT_FMEMOPEN__   (__MPLS_TARGET_OSVER < 101300)

/* pthread_setname_np */
#define __MPLS_SDK_SUPPORT_PTHREAD_SETNAME_NP__   (__MPLS_SDK_MAJOR < 1060)
#define __MPLS_LIB_SUPPORT_PTHREAD_SETNAME_NP__   (__MPLS_TARGET_OSVER < 1060)

/* Compound macros, bundling functionality needed by multiple single features. */
#define __MPLS_SDK_NEED_ATCALL_MACROS__  (__MPLS_SDK_SUPPORT_ATCALLS__ \
                                          || __MPLS_SDK_SUPPORT_SETATTRLISTAT__)

#define __MPLS_SDK_NEED_BEST_FCHDIR__    (__MPLS_SDK_SUPPORT_FDOPENDIR__ \
                                          || __MPLS_SDK_SUPPORT_ATCALLS__ \
                                          || __MPLS_SDK_SUPPORT_SETATTRLISTAT__)
#define __MPLS_LIB_NEED_BEST_FCHDIR__    (__MPLS_LIB_SUPPORT_FDOPENDIR__ \
                                          || __MPLS_LIB_SUPPORT_ATCALLS__ \
                                          || __MPLS_LIB_SUPPORT_SETATTRLISTAT__)

#define __MPLS_LIB_SUPPORT_REALPATH_WRAP__ (__MPLS_LIB_SUPPORT_REALPATH_ALLOC__ \
                                            || __MPLS_LIB_SUPPORT_REALPATH_NONEX_FIX__)

/* UUIDs - for now, just add missing typedef statements */
#define __MPLS_SDK_SUPPORT_UUID__  (__MPLS_SDK_MAJOR < 1060)

/* for now, just forward call to CFPropertyListCreateWithStream */
#define __MPLS_SDK_SUPPORT_CoreFoundation__  (__MPLS_SDK_MAJOR < 1060)

/* copyfile and its associated functions have gained functionality over the years */
#define __MPLS_SDK_SUPPORT_COPYFILE_WRAP__  (__MPLS_SDK_MAJOR < 1060)
#define __MPLS_LIB_SUPPORT_COPYFILE_WRAP__  (__MPLS_TARGET_OSVER < 1060)

/* _tlv_atexit and __cxa_thread_atexit */
#define __MPLS_LIB_SUPPORT_ATEXIT_WRAP__   (__MPLS_TARGET_OSVER < 1070)

/* os_unfair_lock structure and its associated functions */
#define __MPLS_SDK_SUPPORT_OS_UNFAIR_LOCK__   (__MPLS_SDK_MAJOR < 101200)
#define __MPLS_LIB_SUPPORT_OS_UNFAIR_LOCK__   (__MPLS_TARGET_OSVER < 101200)

/* library symbol ___bzero */
#define __MPLS_LIB_SUPPORT_SYMBOL____bzero__   (__MPLS_TARGET_OSVER < 1060)

/* library symbol _dirfd */
#define __MPLS_LIB_SUPPORT_SYMBOL__dirfd__   (__MPLS_TARGET_OSVER < 1080)

/* fix bug in pthread_get_stacksize_np */
/* see https://github.com/rust-lang/rust/issues/43347 */
#define __MPLS_LIB_SUPPORT_PTHREAD_GET_STACKSIZE_NP_FIX__  (__MPLS_TARGET_OSVER == 101000 \
                                                            || __MPLS_TARGET_OSVER == 1090 \
                                                            || __MPLS_TARGET_OSVER <  1060)

#endif /* _MACPORTS_LEGACYSUPPORTDEFS_H_ */
