
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

#ifndef _MACPORTS_TIME_H_
#define _MACPORTS_TIME_H_

// Include the primary system time.h
#include_next <time.h>

// MP support header
#include "MacportsLegacySupport.h"

// Legacy implementation of clock_gettime
#if __MP_LEGACY_SUPPORT_GETTIME__

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  1 // CALENDAR_CLOCK
#define CLOCK_MONOTONIC 0 // SYSTEM_CLOCK
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int clock_gettime( int clk_id, struct timespec *ts );

#ifdef __cplusplus
}
#endif

#endif // _MP_LEGACY_SUPPORT_GETTIME__

#endif // _MACPORTS_TIME_H_
