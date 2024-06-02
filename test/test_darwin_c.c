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

/*
 * This provides tests for the proper effects of __DARWIN_C_LEVEL.
 * Except for the initial macro definition tests, these involve providing
 * definitions which would conflict with the standard headers if the
 * normal declarations/definitions weren't suppressed by __DARWIN_C_LEVEL.
 * Since only one value of __DARWIN_C_LEVEL can be effective in a given build,
 * this entire test is wrapped multiple times with different values.
 * Since there's no easy way to defer such conflicts until runtime, all
 * failures (including the few that could be deferred) manifest themselves
 * as build failures.  The runtime test is just a dummy (almost).
 */

/* First verify that the basic macros are defined. */

#include <sys/cdefs.h>

#ifndef __DARWIN_C_ANSI
#error __DARWIN_C_ANSI is not defined
#endif

#ifndef __DARWIN_C_FULL
#error __DARWIN_C_FULL is not defined
#endif

#ifndef __DARWIN_C_LEVEL
#error __DARWIN_C_LEVEL is not defined
#endif

/* Dummy runtime test, just reports value */

#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("__DARWIN_C_LEVEL = %d\n", (int) __DARWIN_C_LEVEL);
  return 0;
}
