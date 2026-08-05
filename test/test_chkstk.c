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
 * This provides a minimal test of the dummy ____chkstk_darwin() function.
 * Since it's normally called directly by compiler-generated assembly code,
 * and it's not entirely clear that calling it from C is valid, we simply
 * verify the presence of the symbol with an external reference.
 */

#include <libgen.h>
#include <stdio.h>

/* This is only provided for x86 */
#if defined(__i386__) || defined(__x86_64__)

extern void ___chkstk_darwin(void);

typedef void (chkstk_t)(void);

chkstk_t *chkstk_p = ___chkstk_darwin;

#endif  /* x86 */

int
main(int argc, char *argv[])
{
  char *progname = basename(argv[0]);

  (void) argc;

  printf("%s passed.\n", progname);
  return 0;
}
