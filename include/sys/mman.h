
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

#ifndef _MACPORTS_MMAN_H_
#define _MACPORTS_MMAN_H_

// Include the primary system sys/mman.h
#include_next <sys/mman.h>

// MAP_ANONYMOUS only exists on 10.11+
// Prior to that it was called MAP_ANON
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif /* _MACPORTS_MMAN_H_ */
