/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

#ifndef _MACPORTS_SYS_QUEUE_H_
#define _MACPORTS_SYS_QUEUE_H_

/* Include the primary system sys/queue.h */
#include_next <sys/queue.h>

/* SLIST functions missing from earlier SDK versions */

/* Missing until 10.5 */

#ifndef SLIST_HEAD_INITIALIZER
#define	SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }
#endif

/* Missing until 10.7 */

#ifndef SLIST_REMOVE_AFTER
#define SLIST_REMOVE_AFTER(elm, field) do {             \
        SLIST_NEXT(elm, field) =                        \
            SLIST_NEXT(SLIST_NEXT(elm, field), field);  \
} while (0)
#endif

/* STAILQ functions missing from earlier SDK versions */

/* Missing until 10.5 */

#ifndef STAILQ_FOREACH
#define STAILQ_FOREACH(var, head, field)     \
    for((var) = STAILQ_FIRST((head));        \
       (var);                                \
       (var) = STAILQ_NEXT((var), field))
#endif

#endif /* _MACPORTS_SYS_QUEUE_H_ */
