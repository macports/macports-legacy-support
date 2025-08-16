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
 * It's also been discovered that it's a bit flaky on 11.x, at least on arm64
 * machines, including when running x86_64 under Rosetta 2.  So we exempt that
 * as well.
 *
 * For simplicity, the OS version test is based on the build-target version
 * rather than checking the actual version at runtime.  This means that
 * running this test on a different OS version than the one it was built
 * for may not have the desired result.
 */

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#define TARGET_OS __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define TARGET_OS 1040
#endif

#if TARGET_OS >= 101500 && TARGET_OS < 120000

#if TARGET_OS < 110000
#define VERSION "10.15"
#define SYMPTOM "brokenness"
#else
#define VERSION "11"
#define SYMPTOM "flakiness"
#endif

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s is being skipped on macOS " VERSION " due to OS " SYMPTOM ".\n",
         basename(argv[0]));
  return 0;
}

#else /* Not 10.15 or 11.x */

#define _POSIX_C_SOURCE 200112L

#include "test_realpath.c"

#endif /* Not 10.15 or 11.x */
