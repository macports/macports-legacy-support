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
 * This provides tests to verify that certain symbol aliases are present.
 *
 * Although there is probably a way to exploit weak references to defer
 * errors until runtime, there are some complications with that approach,
 * so we settle for simple external references, where failures are build
 * errors rather than runtime errors.
 */

#include <stdio.h>
#include <string.h>

/* Macro to avoid warnings converting pointer to ULL */
#if defined(__LP64__) && __LP64__
#define PTR2ULL(x) ((unsigned long long) (x))
#else
#define PTR2ULL(x) ((unsigned long long) (unsigned int) (x))
#endif

#define CHECK_SYMS \
  SYM_MAC(__bzero) \
  SYM_MAC(dirfd)

#define SYM_MAC(name) \
  extern void name(); \
  void *name##_adr = &name;
CHECK_SYMS
#undef SYM_MAC

int
main(int argc, char *argv[])
{
  int verbose = 0;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) {
    #define SYM_MAC(name) \
      printf(" " #name " = %llX\n", PTR2ULL(name##_adr));
    CHECK_SYMS
    #undef SYM_MAC
  }

  printf("symbol aliases test passed.\n");
  return 0;
}
