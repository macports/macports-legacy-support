
/* Copyright (C) 1996-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// MP support header
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_STRNDUP__

#ifndef HAVE_STRNDUP

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "strnlen.h"

char *
strndup (const char *s, size_t n)
{
  size_t len = strnlen (s, n);
  char *new = (char *) malloc (len + 1);

  if (new == NULL)
    return NULL;

  new[len] = '\0';
  return (char *) memcpy (new, s, len);
}

#endif /* HAVE_STRNDUP */

#endif /* __MP_LEGACY_SUPPORT_STRDUP__ */
