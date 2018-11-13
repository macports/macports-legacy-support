
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

#ifndef _MACPORTS_SYSFCNTL_H_
#define _MACPORTS_SYSFCNTL_H_

/* Include the primary system wchar.h */
#include_next <sys/fcntl.h>

/* replace missing O_CLOEXEC definition with 0, which works
 * but does not replace the full function of that flag
 * this is the commonly done fix in MacPorts (see gtk3, for example) 
 */

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#endif /* _MACPORTS_SYSFCNTL_H_ */
