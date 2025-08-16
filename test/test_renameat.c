/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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
 * Currently this is just an "almost dummy" test, just to verify that
 * renameat() is declared in the expected header and defined in the
 * library (if needed).
 *
 * The declaration of renameat() is actually in sys/stdio.h, but that's
 * included by stdio.h, so we test it there.
 */

#include <stdio.h>

/*
 * This is *not* static, to keep it from being optimized out, and thereby
 * forcing a reference to the library or system renameat().
 */
int
our_renameat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath)
{
  return renameat(olddirfd, oldpath, newdirfd, newpath);
}

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  (void) our_renameat;

  return 0;
}
