
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

#ifndef _MACPORTS_STRING_H_
#define _MACPORTS_STRING_H_

// Include the primary system string.h
#include_next <string.h>

// MP support header
#include "MacportsLegacySupport.h"

// strnlen
#if __MP_LEGACY_SUPPORT_STRNLEN__
#ifdef __cplusplus
extern "C" {
#endif
  extern size_t strnlen(const char *s, size_t maxlen);
#ifdef __cplusplus
}
#endif
#endif

// strndup
#if __MP_LEGACY_SUPPORT_STRNDUP__
#ifdef __cplusplus
extern "C" {
#endif
  extern char *strndup(const char *s, size_t n);
#ifdef __cplusplus
}
#endif
#endif

// memmem
#if __MP_LEGACY_SUPPORT_MEMMEM__
#ifdef __cplusplus
extern "C" {
#endif
  extern void *
  memmem(const void *l, size_t l_len, const void *s, size_t s_len);
#ifdef __cplusplus
}
#endif
#endif

#endif // _MACPORTS_STRING_H_
