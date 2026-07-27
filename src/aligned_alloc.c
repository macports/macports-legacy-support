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
 * This implements the aligned_alloc() function, which is similar to
 * posix_memalign().  The only reason it's in a separate source is that
 * the latter is a modified Apple source, and this keeps the non-Apple
 * code separate.
 */

/* MP support header */
#include "MacportsLegacySupport.h"
#if __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

void *
aligned_alloc(size_t alignment, size_t size)
{
  void *result;

  /* This function requires the size to be a multiple of the alignment. */
  /* Other restrictions are provided by posix_memalign() */
  if (size & (alignment - 1)) {
    errno = EINVAL;
    return NULL;
  }

  if (posix_memalign(&result, alignment, size)) return NULL;
  return result;
}

#endif  /* __MPLS_LIB_SUPPORT_ALIGNED_ALLOC__ */
