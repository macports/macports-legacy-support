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
 * An earlier implementation of fdopendir() defined seekdir as a macro,
 * which originally conflicted with a C++ library definition.  It was fixed
 * to avoid that, and the test that this replaces was written in C++ in
 * order to verify the fix.  But C++ tests have become more problematic
 * as of the 15.x SDK, and meanwhile, the fdopendir() implementation was
 * rewritten to use a different approach, making the old test moot.
 *
 * This test is now just a dummy to verify the include of dirent.h.
 * The original C++ test still exists, but now as:
 *   manual_tests/dirent_with_cplusplus.cpp
 */

#include <dirent.h>
#include <stdio.h>
 
int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("dirent.h successfully included\n");

  return 0;
}
