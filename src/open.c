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

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_OPEN_CLOEXEC__

/*
 * O_CLOEXEC isn't available as an open() option prior to 10.7, though
 * the feature is accessible via fcntl().  Here we provide a wrapper to
 * implement the former via the latter.
 *
 * Since open() has multiple variants, we provide corresponding wrappers.
 */

/* Make sure we get "bare" open() name */
#define __DARWIN_UNIX03 0

#include <fcntl.h>
#include <stdarg.h>

#include <sys/fcntl.h>

#include "util.h"

DEFINE_MPLS_ABORTMSG

typedef int (open_fn_t)(const char *, int, ...);

static int
open_internal(open_fn_t *open_fn, const char *path, int oflag, va_list ap)
{
  int fd;
  mode_t mode = 0;

  /* Handle optional mode arg */
  if (oflag & O_CREAT) {
    mode = va_arg(ap, int);  /* mode_t will be promoted to int */
  }

  /* Do the open() without O_CLOEXEC */
  if (oflag & O_CREAT) {
    fd = (*open_fn)(path, oflag &~O_CLOEXEC, mode);
  } else {
    fd = (*open_fn)(path, oflag &~O_CLOEXEC);
  }

  /*
   * If requested (and the open() succeeded), now set CLOEXEC.
   * Ignore failure, since that's probably better than failing the open()
   * and is consistent with the old behavior of defining O_CLOEXEC as 0.
   */
  if ( fd >=0 && oflag & O_CLOEXEC) {
    (void) fcntl(fd, F_SETFD, 1);
  }

  return fd;
}

int
open(const char *path, int oflag, ...)
{
  int fd;
  va_list ap;
  GET_OS_FUNC(open)

  va_start(ap, oflag);
  fd = open_internal(os_open, path, oflag, ap);
  va_end(ap);

  return fd;
}

#ifdef __LP64__

int
open$NOCANCEL(const char *path, int oflag, ...)
{
  int fd;
  va_list ap;
  GET_OS_OPT_ALT_FUNC(open, $NOCANCEL)

  va_start(ap, oflag);
  fd = open_internal(os_open, path, oflag, ap);
  va_end(ap);

  return fd;
}

#else  /* !__LP64__ */

int
open$UNIX2003(const char *path, int oflag, ...)
{
  int fd;
  va_list ap;
  GET_OS_OPT_ALT_FUNC(open, $UNIX2003)

  va_start(ap, oflag);
  fd = open_internal(os_open, path, oflag, ap);
  va_end(ap);

  return fd;
}

int
open$NOCANCEL$UNIX2003(const char *path, int oflag, ...)
{
  int fd;
  va_list ap;
  GET_OS_OPT_ALT_FUNC(open, $NOCANCEL$UNIX2003)

  va_start(ap, oflag);
  fd = open_internal(os_open, path, oflag, ap);
  va_end(ap);

  return fd;
}

#endif  /* !__LP64__ */

#endif  /* __MPLS_LIB_SUPPORT_OPEN_CLOEXEC__ */
