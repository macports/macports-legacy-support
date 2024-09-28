/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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
#if __MPLS_LIB_SUPPORT_DPRINTF__

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

int
dprintf(int fildes, const char * __restrict format, ...) {
  va_list ap;
  FILE *stream;
  int ret;
  char buf[BUFSIZ];

  /* Create a stream for a copy of the target fd, with our local buffer. */
  stream = fdopen(dup(fildes), "w");
  if (stream == NULL) {
    errno = EBADF;  /* Set the expected errno if it fails */
    return -1;
  }
  setbuffer(stream, buf, sizeof(buf));

  /* Do the output. */
  va_start(ap, format);
  ret = vfprintf(stream, format, ap);
  va_end(ap);

  /*
   * Close the FILE and the duplicate fd.
   *
   * NOTE: This releases any locks held by the original fd.  There doesn't
   * seem to be an easy way to avoid this, given that free(stream) doesn't
   * work.
   */
  if (fclose(stream)) ret = -1;

  return ret;
}

#endif /* __MPLS_LIB_SUPPORT_DPRINTF__ */
