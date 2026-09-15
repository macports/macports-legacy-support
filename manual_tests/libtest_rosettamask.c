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
 * This provides a "test" to report the current Rosetta bug mask, if any.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <_macports_extras/targetos.h>

#if defined(__ppc__)
#define BUG_MASK __mpls_rosetta1_bugs
#elif defined(__x86_64__) && __MPLS_TARGET_OSVER >= 110000
#define BUG_MASK __mpls_rosetta2_bugs
#endif

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

#ifdef BUG_MASK
  extern uint64_t BUG_MASK;
  printf("Rosetta bug mask = 0x%llX\n", (unsigned long long) BUG_MASK);
#else
  printf("No Rosetta bug mask on this system\n");
#endif

  return 0;
}
