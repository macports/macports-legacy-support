/*
 * Copyright (c) 2019 Ken Cunningham <kencu@macports.org>
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

/* MP support header */
#include "MacportsLegacySupport.h"

/* realpath wrap */
#if __MP_LEGACY_SUPPORT_REALPATH_WRAP__

/* we need this blocker so as to not get caught in our own wrap */
#undef __MP_LEGACY_SUPPORT_REALPATH_WRAP__
#define __DISABLE_MP_LEGACY_SUPPORT_REALPATH_WRAP__ 

#include <limits.h>
#include <stdlib.h>

char *
macports_legacy_realpath(const char * __restrict stringsearch, char * __restrict buffer)
{
    if (buffer == NULL) {
        char *myrealpathbuf = malloc(PATH_MAX);
        if (myrealpathbuf != NULL) {
            return(realpath(stringsearch, myrealpathbuf));
        } else {
            return(NULL);
        }
    } else {
        return(realpath(stringsearch, buffer));
    }
}
#endif /*__MP_LEGACY_SUPPORT_REALPATH_WRAP__*/
