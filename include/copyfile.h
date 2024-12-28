/*
 * Copyright (c) 2024
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

#ifndef _MACPORTS_COPYFILE_H_
#define _MACPORTS_COPYFILE_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_SUPPORT_COPYFILE_TIGER__

/* The 10.4 SDK lacks copyfile.h, so we provide a substitute. */
#include <_macports_extras/tiger_only/copyfile.h>

/* Additional defs from the 10.5 copyfile.h */

__MP__BEGIN_DECLS

int fcopyfile(int from_fd, int to_fd, copyfile_state_t, copyfile_flags_t flags);

int copyfile_state_free(copyfile_state_t);
copyfile_state_t copyfile_state_alloc(void);

int copyfile_state_get(copyfile_state_t s, uint32_t flag, void * dst);
int copyfile_state_set(copyfile_state_t s, uint32_t flag, const void * src);

__MP__END_DECLS

#define COPYFILE_STATE_SRC_FD		1
#define COPYFILE_STATE_SRC_FILENAME	2
#define COPYFILE_STATE_DST_FD		3
#define COPYFILE_STATE_DST_FILENAME	4
#define COPYFILE_STATE_QUARANTINE	5

#undef COPYFILE_DISABLE_VAR
#define	COPYFILE_DISABLE_VAR	"COPYFILE_DISABLE"

#else /* !__MPLS_SDK_SUPPORT_COPYFILE_TIGER__ */

/* Otherwise include the primary system copyfile.h */
#include_next <copyfile.h>

#endif /* !__MPLS_SDK_SUPPORT_COPYFILE_TIGER__ */

#if __MPLS_SDK_SUPPORT_COPYFILE_10_6__

/* Additional defs from the 10.6 copyfile.h */

typedef int (*copyfile_callback_t)(int, int, copyfile_state_t, const char *, const char *, void *);

#define COPYFILE_STATE_STATUS_CB        6
#define COPYFILE_STATE_STATUS_CTX       7
#define COPYFILE_STATE_COPIED           8

#define	COPYFILE_RECURSIVE	(1<<15)	/* Descend into hierarchies */

#define	COPYFILE_RECURSE_ERROR	0
#define	COPYFILE_RECURSE_FILE	1
#define	COPYFILE_RECURSE_DIR	2
#define	COPYFILE_RECURSE_DIR_CLEANUP	3
#define	COPYFILE_COPY_DATA	4

#define	COPYFILE_START		1
#define	COPYFILE_FINISH		2
#define	COPYFILE_ERR		3
#define	COPYFILE_PROGRESS	4

#define	COPYFILE_CONTINUE	0
#define	COPYFILE_SKIP	1
#define	COPYFILE_QUIT	2

#endif /* __MPLS_SDK_SUPPORT_COPYFILE_10_6__ */

#endif /* _MACPORTS_COPYFILE_H_ */
