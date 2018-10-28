
/*
 * Copyright (c) 2010 Chris Jones <jonesc@macports.org>
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

#include "AvailabilityMacros.h"

// defines for when legacy support is required for various functions

// clock_gettime
#define __MP_LEGACY_SUPPORT_GETTIME__ __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200

// strnlen
#define __MP_LEGACY_SUPPORT_STRNLEN__ __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

// strndup
#define __MP_LEGACY_SUPPORT_STRNDUP__ __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

// getline
#define __MP_LEGACY_SUPPORT_GETLINE__ __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

// memmem
#define __MP_LEGACY_SUPPORT_MEMMEM__  __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

// wcsdup
#define __MP_LEGACY_SUPPORT_WCSDUP__  __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

// llround
#define __MP_LEGACY_SUPPORT_LLROUND__  __APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070

#endif /* _MACPORTS_LEGACYSUPPORTDEFS_H_ */