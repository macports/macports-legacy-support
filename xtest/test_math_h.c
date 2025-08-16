/* Simple test to verify that math.h can be compiled. */

#include <_macports_extras/sdkversion.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define TARGET_OSVER __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define TARGET_OSVER 1040
#endif

#if defined(__clang_major__)
#define COMPILER "clang %d.%d", __clang_major__, __clang_minor__
#elif defined(__GNUC__)
#define COMPILER "gcc %d.%d", __GNUC__, __GNUC_MINOR__
#else
#define COMPILER "<unknown>"
#endif

#if defined(__APPLE_CC__)
#define COMPILER_APPLE " (Apple %d)\n", __APPLE_CC__
#else
#define COMPILER_APPLE "\n"
#endif

#if defined(__x86_64__)
#define ARCH "x86_64"
#elif defined(__arm64__)
#define ARCH "arm64"
#else
#define ARCH "non-_Float16"
#endif

static void
print_basic(void)
{
  #if __MPLS_SDK_MAJOR < 150000
    printf("No _Float16 workaround needed.\n");
  #elif !defined(_Float16)
    printf("math.h builds without _Float16 workaround\n");
  #else
    printf("math.h builds with _Float16 defined as a macro\n");
  #endif
}

static
void
print_details(void)
{
  printf("Target OS is %d\n", TARGET_OSVER);
  printf("SDK major version is %d\n", __MPLS_SDK_MAJOR);
  printf("Compiler is " COMPILER);
  printf(COMPILER_APPLE);
  printf("Architecture is " ARCH "\n");
  printf("__MPLS_FLOAT16_STATUS = %d\n", __MPLS_FLOAT16_STATUS);
}

int
main(int argc, char *argv[])
{
  int verbose = 0;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  print_basic();

  if (verbose) {
    print_details();
    printf("\n");
  }

  return 0;
}
