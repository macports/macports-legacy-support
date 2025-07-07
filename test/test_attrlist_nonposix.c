/*
 * Version of test_realpath with non-POSIX semantics (32-bit only).
 *
 * Attempting a 64-bit build with _NONSTD_SOURCE results in an error.
 */

#if defined(__LP64__) && __LP64__

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s is inapplicable.\n", basename(argv[0]));
  return 0;
}

#else /* 32-bit */

#define _NONSTD_SOURCE
#include "test_attrlist.c"

#endif /* 32-bit */
