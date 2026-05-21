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
 * This test verifies that xnu_static_assert_struct_size() defined in the SDK 26
 * mach/port.h can be successfully used with compilers that don't support
 * _Static_assert().
 */

#include <libgen.h>
#include <stdio.h>

#include <mach/port.h>

int
main(int argc, char *argv[])
{
  (void) argc;

  #ifdef xnu_static_assert_struct_size
    xnu_static_assert_struct_size(char, sizeof(char));
  #endif

  printf("%s succeeded.\n", basename(argv[0]));
  return 0;
}
