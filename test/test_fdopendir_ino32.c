/*
 * Version of test_fdopendir with 32-bit inodes (if possible).
 *
 * This doesn't work on arm64.
 */

#if defined(__arm64__) && __arm64__

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s doesn't work on arm64.\n", basename(argv[0]));
  return 0;
}

#else /* !arm64 */

#define _DARWIN_NO_64_BIT_INODE 1

#include "test_fdopendir.c"

#endif /* !arm64 */
