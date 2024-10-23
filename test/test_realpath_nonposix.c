/*
 * Version of test_realpath with non-POSIX semantics (32-bit only).
 */

#if !defined(__LP64__) || !__LP64__

#define _NONSTD_SOURCE
#include "test_realpath.c"

#else

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s is inapplicable.\n", basename(argv[0]));
  return 0;
}

#endif
