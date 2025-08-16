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
 * This test is just a minimal test to verify the added CPU_* definitions
 * in mach/machine.h.
 */

#include <libgen.h>
#include <stdio.h>
#include <string.h>

#include <mach/machine.h>

#define PRINT_VAR(x) if (verbose) printf("%s = %d\n", #x, x);
#define PRINT_UNDEF(x) printf(#x " is undefined\n"); ret = 1;

int
main(int argc, char *argv[])
{
  int ret = 0, verbose = 0;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  #ifdef CPU_TYPE_ARM
  PRINT_VAR(CPU_TYPE_ARM);
  #else
  PRINT_UNDEF(CPU_TYPE_ARM);
  #endif

  #ifdef CPU_SUBTYPE_ARM64E
  PRINT_VAR(CPU_SUBTYPE_ARM64E);
  #else
  PRINT_UNDEF(CPU_SUBTYPE_ARM64E);
  #endif

  printf("%s %s.\n", basename(argv[0]), ret ? "failed" : "passed");
  return ret;
}
