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
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

/*
 * KLUDGE: Arrange to disable underlying close() in fclose().
 *
 * If we simply use fdopen() and then fclose() to establish the temporary
 * stream, the fclose() issues a close() on the underlying fd, preventing
 * its further use.  We can mostly avoid this (the earlier approach) by
 * using dup() to create a duplicate fd that can mostly be safely closed,
 * but since the duplicate fd shares any locks with the original, the close()
 * still releases the locks, possibly causing weird failures.
 *
 * Apple's library code avoids this by creating a FILE as an auto variable
 * within the function, and appropriately initializing it to use the provided
 * fd.  It finishes with an fflush() but not a close(), thereby leaving the
 * fd unperturbed except for the added data.  But this approach relies on
 * knowledge of the private internal layout of the FILE, which is not
 * available to code outside the system libraries.
 *
 * Without substantial internal knowledge, there's no way to create a FILE
 * without resorting to fdopen().  And since fdopen() uses a special
 * allocator, there's no way (again without private knowledge) to free
 * the resulting FILE other than with fclose().
 *
 * Another approach which requires considerably less (but still nonzero)
 * internal knowledge is to set the internal '_close' function pointer to
 * NULL, which disables the close() while allowing the rest of fclose()
 * to operate normally (including the important fflush()).
 *
 * The issue with this approach is knowing the offset for the _close element.
 * Fortunately, a few things work in our favor:
 *   1) The '_close' element is immediately preceded by a '_cookie' element,
 *      which is actually a self pointer to the FILE itself.  This allows
 *      a fairly reliable consistency check.
 *   2) The layout of FILE (at least up through '_close') has never changed
 *      across all the OS versions, avoiding the need for version conditions.
 *   3) This code is only applicable to OS versions 10.6 and earlier, which
 *      have been frozen for ages, avoiding any future compatibility issues.
 *
 * So what we do here is to define enough of the FILE layout to cover the
 * fields of interest (skipping the rest), use a check on the '_cookie'
 * element as a sanity check, and then (if it passes) set the '_close'
 * element to NULL.  If the sanity check fails, we leave '_close' alone,
 * which reintroduces the unwanted close() of the fd, but that's a more
 * obvious failure than releasing locks (and is tested by the accompanying
 * tests).
 *
 * Note that Apple didn't get around to hiding the private definitions
 * until the 11.x SDK, so we prefix our versions with '__mpls' to avoid
 * conflicts.
 */

/* stdio buffers */
struct __mpls__sbuf {
	unsigned char	*_base;
	int		_size;
};

/* stdio FILE object (truncated) */
struct __mpls__sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
	short	_file;		/* fileno, if Unix descriptor, else -1 */
	struct	__mpls__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;	/* cookie passed to io functions */
	int	(*_close)(void *);
	/* We don't need the rest */
};

int
vdprintf(int fildes, const char * __restrict format, va_list ap) {
  FILE *stream;
  struct __mpls__sFILE *filep;
  int ret;
  char buf[BUFSIZ];

  /* Create a stream for the target fd, with our local buffer. */
  stream = fdopen(fildes, "w");
  if (stream == NULL) {
    errno = EBADF;  /* Set the expected errno if it fails */
    return -1;
  }
  setbuffer(stream, buf, sizeof(buf));

  /* If the FILE looks as expected, clear the _close pointer. */
  filep = (struct __mpls__sFILE *) stream;
  if (filep->_cookie == filep) filep->_close = NULL;

  /* Do the output. */
  ret = vfprintf(stream, format, ap);

  /* Close the FILE (but not the fd). */
  if (fclose(stream)) ret = -1;

  return ret;
}

int
dprintf(int fildes, const char * __restrict format, ...) {
  va_list ap;
  int ret;

  va_start(ap, format);
  ret = vdprintf(fildes, format, ap);
  va_end(ap);

  return ret;
}

#endif /* __MPLS_LIB_SUPPORT_DPRINTF__ */
