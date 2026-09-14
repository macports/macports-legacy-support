/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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
 * though this isn't normally necessary.  In particular, overriding the
 * default is *not* necessary just to build the library for an alternate
 * target OS; that should be done via the usual target settings, which will
 * in turn be picked up by the definition here.
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
 * Major versions of libSystem (obtainable via NSVersionOfRunTimeLibrary)
 * corresponding to OS versions.  Note that the earliest OS where this is
 * available is 10.5; prior versions return -1.
 */
#define __MPLS_SYSLIB_NAME     "System"
#define __MPLS_LIBMAJOR_SHIFT  16
#define __MPLS_SYSLIB_1050      (111 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_1060      (123 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_1070      (159 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_1080      (169 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_1090     (1197 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101000   (1213 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101100   (1225 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101200   (1238 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101300   (1252 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101400   (1252 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_101500   (1281 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_110000   (1292 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_120000   (1311 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_130000   (1319 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_140000   (1336 << __MPLS_LIBMAJOR_SHIFT)
#define __MPLS_SYSLIB_150000   (1351 << __MPLS_LIBMAJOR_SHIFT)
