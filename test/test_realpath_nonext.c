/*
 * Version of test_realpath with Darwin extensions disabled.
 */

/*
 * NOTE: This version of realpath() is completely broken on 10.15.
 * This is not the fault of legacy-support, since we don't currently modify
 * the OS realpath() behavior on 10.6+.  Unless and until we provide a fix
 * for that OS bug, we need to carve out an exemption in this test for 10.15,
 * to avoid an undeserved test failure.
 *
 * For simplicity, the OS version test is based on the build-target version
 * rather than checking the actual version at runtime.  This means that
 * running this test on a different OS version than the one it was built
 * for may not have the desired result.
 */

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 101500 \
    && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ <  110000

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s is being skipped on 10.15 due to OS brokenness.\n",
         basename(argv[0]));
  return 0;
}

#else /* Not 10.15 */

#define _POSIX_C_SOURCE 200112L

#include "test_realpath.c"

#endif /* Not 10.15 */
