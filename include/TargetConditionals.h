/*
 * Copyright (c) 2025
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

#ifndef __MPLS_TARGETCONDITIONALS__
#define __MPLS_TARGETCONDITIONALS__

/*
 * There are instances in the Apple header of the forms:
 *   #if !defined(__has_extension) || !__has_extension(define_target_os_macros)
 *   #if defined(__has_builtin) && __has_builtin(__is_target_arch)
 * This doesn't work when the first macro is undefined, since the second
 * macro is only correctly parseable when the first macro is defined.
 * This causes failures when building with GCC 4.2.  We get around it
 * by temporarily defining the first macro as a dummy during the include_next
 * in this situation.
 *
 * Since TargetConditionals.h doesn't include any other headers, this hack
 * only applies to its own processing.
 */

#ifndef __has_extension
#define __MPLS_HAS_EXTENSION_UNDEF
#define __has_extension(x) 0
#endif

#ifndef __has_builtin
#define __MPLS_HAS_BUILTIN_UNDEF
#define __has_builtin(x) 0
#endif

#include_next <TargetConditionals.h>

#ifdef __MPLS_HAS_EXTENSION_UNDEF
#undef __MPLS_HAS_EXTENSION_UNDEF
#undef __has_extension
#endif

#ifdef __MPLS_HAS_BUILTIN_UNDEF
#undef __MPLS_HAS_BUILTIN_UNDEF
#undef __has_builtin
#endif

/*
 * Provide defaults for target macros not defined in earlier SDKs.
 * Defaulting to zero is OK in almost all cases, since anything which needs
 * to be nonzero will have been defined nonzero.  The one exception is
 * TARGET_OS_OSX, which is sometimes undefined, or even sometimes defined
 * as 0, so we unconditionally define it as 1 here.
 *
 * This list is copied almost verbatim (except for the TARGET_OS_OSX definition
 * as noted) from the SDK 15 TargetConditionals.h
 */

#ifndef TARGET_OS_MAC
    #define TARGET_OS_MAC        0
#endif

#undef TARGET_OS_OSX
    #define TARGET_OS_OSX        1

#ifndef TARGET_OS_IPHONE
    #define TARGET_OS_IPHONE     0
#endif

#ifndef TARGET_OS_IOS
    #define TARGET_OS_IOS        0
#endif

#ifndef TARGET_OS_WATCH
    #define TARGET_OS_WATCH      0
#endif

#ifndef TARGET_OS_TV
    #define TARGET_OS_TV         0
#endif

#ifndef TARGET_OS_SIMULATOR
    #define TARGET_OS_SIMULATOR  0
#endif

#ifndef TARGET_OS_EMBEDDED
    #define TARGET_OS_EMBEDDED   0
#endif

#ifndef TARGET_OS_RTKIT
    #define TARGET_OS_RTKIT      0
#endif

#ifndef TARGET_OS_MACCATALYST
    #define TARGET_OS_MACCATALYST 0
#endif

#ifndef TARGET_OS_VISION
    #define TARGET_OS_VISION     0
#endif

#ifndef TARGET_OS_UIKITFORMAC
    #define TARGET_OS_UIKITFORMAC 0
#endif

#ifndef TARGET_OS_DRIVERKIT
    #define TARGET_OS_DRIVERKIT 0
#endif 

#ifndef TARGET_OS_WIN32
    #define TARGET_OS_WIN32     0
#endif

#ifndef TARGET_OS_WINDOWS
    #define TARGET_OS_WINDOWS   0
#endif



#ifndef TARGET_OS_LINUX
    #define TARGET_OS_LINUX     0
#endif

#ifndef TARGET_CPU_PPC
    #define TARGET_CPU_PPC      0
#endif

#ifndef TARGET_CPU_PPC64
    #define TARGET_CPU_PPC64    0
#endif

#ifndef TARGET_CPU_68K
    #define TARGET_CPU_68K      0
#endif

#ifndef TARGET_CPU_X86
    #define TARGET_CPU_X86      0
#endif

#ifndef TARGET_CPU_X86_64
    #define TARGET_CPU_X86_64   0
#endif

#ifndef TARGET_CPU_ARM
    #define TARGET_CPU_ARM      0
#endif

#ifndef TARGET_CPU_ARM64
    #define TARGET_CPU_ARM64    0
#endif

#ifndef TARGET_CPU_MIPS
    #define TARGET_CPU_MIPS     0
#endif

#ifndef TARGET_CPU_SPARC
    #define TARGET_CPU_SPARC    0
#endif

#ifndef TARGET_CPU_ALPHA
    #define TARGET_CPU_ALPHA    0
#endif

#ifndef TARGET_ABI_USES_IOS_VALUES
    #define TARGET_ABI_USES_IOS_VALUES  (!TARGET_CPU_X86_64 || (TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST))
#endif

#ifndef TARGET_IPHONE_SIMULATOR
    #define TARGET_IPHONE_SIMULATOR     TARGET_OS_SIMULATOR /* deprecated */
#endif

#ifndef TARGET_OS_NANO
    #define TARGET_OS_NANO              TARGET_OS_WATCH /* deprecated */
#endif

#endif /* __MPLS_TARGETCONDITIONALS__ */
