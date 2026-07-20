/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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
 * Definitions related to tracking whether running under Rosetta, and which
 * Rosetta bugs are currently applicable.  The framework supports both
 * Rosetta 1 and Rosetta 2, but since we don't currently provide enhancements
 * for any OS new enough for Rosetta 2, only the Rosetta 1 parts are currently
 * used.
 */

#ifndef _MACPORTS_ROSETTA_H_
#define _MACPORTS_ROSETTA_H_

/* MP support header */
#include "MacportsLegacySupport.h"

#include <stdint.h>

#if __MPLS_LIB_ROSETTA1_HANDLING__ || __MPLS_LIB_ROSETTA2_HANDLING__

/* -1 = uninit, 0 = native, 1 = Rosetta 1, 2 = Rosetta 2 */
extern int __mpls_is_rosetta;

extern void __mpls_setup_rosetta(void);

#endif  /* __MPLS_LIB_ROSETTA1_HANDLING__ || __MPLS_LIB_ROSETTA2_HANDLING__ */

#if __MPLS_LIB_ROSETTA1_HANDLING__

/* Byte-swapped packet timestamps */
#define _ROSETTA1_BUG_PACKET_TIMESTAMP  (1<<0)

/* Byte-swapped fd in fstatx_np() with non-NULL fsec (10.4 only) */
#define _ROSETTA1_BUG_FSTATX_FD         (1<<1)

/* Byte-swapped fd in fchmodx_np() (10.4 only) */
#define _ROSETTA1_BUG_FCHMODX_FD        (1<<2)

/* Clobbered upper half of kernel threadid from syscall */
#define _ROSETTA1_BUG_GARBLED_THREADID  (1<<3)

/* Word-swapped kernel threadid from syscall */
#define _ROSETTA1_BUG_SWAPPED_THREADID  (1<<4)

/* Bad SIGSYS handling that crashes process */
#define _ROSETTA1_BUG_SIGSYS_CRASH      (1<<5)

/* Garbled machine context with SA_SIGINFO */
#define _ROSETTA1_BUG_GARBLED_CONTEXT   (1<<6)

/* All  bug flags */
#define _ROSETTA1_BUGS_ALL      (_ROSETTA1_BUG_PACKET_TIMESTAMP \
                                 | _ROSETTA1_BUG_FSTATX_FD \
                                 | _ROSETTA1_BUG_FCHMODX_FD \
                                 | _ROSETTA1_BUG_GARBLED_THREADID \
                                 | _ROSETTA1_BUG_SWAPPED_THREADID \
                                 | _ROSETTA1_BUG_SIGSYS_CRASH \
                                 | _ROSETTA1_BUG_GARBLED_CONTEXT)

/* Bugs limited to 10.4 */
#define _ROSETTA1_BUGS_TIGER    (_ROSETTA1_BUG_FSTATX_FD \
                                 | _ROSETTA1_BUG_FCHMODX_FD)

extern uint64_t __mpls_rosetta1_bugs;

#endif  /* __MPLS_LIB_ROSETTA1_HANDLING__ */

#if __MPLS_LIB_ROSETTA2_HANDLING__

/* Bad thread-time scaling */
#define _ROSETTA2_BUG_BAD_THREAD_TIME   (1<<0)

/* Bad packet timestamp scaling */
#define _ROSETTA2_BUG_BAD_PACKET_TIME   (1<<1)

/* All  bug flags */
#define _ROSETTA2_BUGS_ALL      (_ROSETTA2_BUG_BAD_THREAD_TIME \
                                 | _ROSETTA2_BUG_BAD_PACKET_TIME)

extern uint64_t __mpls_rosetta2_bugs;

#endif  /* __MPLS_LIB_ROSETTA2_HANDLING__ */

#endif /* _MACPORTS_ROSETTA_H_ */
