/*
 * Version of test_fdopendir with 32-bit inodes (if possible).
 *
 * This only works on ppc/x86.  Later platforms (e.g. arm64) only
 * support 64-bit inodes.
 */

#if !defined(__ppc__) && !defined(__ppc64__) \
    && !defined(__i386__) && !defined(__x86_64__)

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s is only supported on ppc/x86.\n", basename(argv[0]));
  return 0;
}

#else /* ppc/x86 */

#define _DARWIN_NO_64_BIT_INODE 1

#include "test_fdopendir.c"

#endif /* ppc/x86 */
