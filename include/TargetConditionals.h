/*
 * Copyright (c) 2021 Ken Cunningham <kencu@macports.org>
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
 * TARGET_OS_SIMULATOR replaced TARGET_IPHONE_SIMULATOR and is not defined in older os versions
 */

#include_next <TargetConditionals.h>

#ifndef TARGET_OS_SIMULATOR
#  define TARGET_OS_SIMULATOR         0
#endif

#ifndef TARGET_OS_OSX
#  define TARGET_OS_OSX               1
#endif

/* If not set, will never be true */
#ifndef TARGET_CPU_ARM
#  define TARGET_CPU_ARM              0
#endif
#ifndef TARGET_CPU_ARM64
#  define TARGET_CPU_ARM64            0
#endif
