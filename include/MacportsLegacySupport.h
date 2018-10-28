
/* Copyright (C) 1994, 1996, 1997, 1998, 2001, 2003, 2005, 2006 Free
   Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

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
