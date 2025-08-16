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
 * This is a test which includes all wrapper headers that we provide, in
 * order to verify that they can be compiled with a variety of compiler
 * versions, compiler options, and feature flags.  The runtime "test"
 * is just a dummy.
 */

#define __MPLS_HEADER_TEST__

#include "allheaders.h"

/* Explicitly include the headers we actually use here */
#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("%s succeeded.\n", basename(argv[0]));
  return 0;
}
