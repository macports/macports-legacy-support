/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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
 * This is a test to check that various target-conditional macros are defined.
 *
 * It also (indirectly) tests the behavior of the workaround for broken
 * __is_target_environment() implementations, which otherwise cause trouble
 * with SDK 27+.
 *
 * It also reports the "raw" state of __is_target_environment() in verbose
 * mode.
 */

int printf(const char *, ...);  /* Needed before includes */

#define PRINT_VAR(x) if (verbose) printf("%s = %d\n", #x, x);
#define PRINT_UNDEF(x) printf(#x " is undefined\n"); ret = 1;

#define PRINT_VAR_NEEDNZ(x) \
  if (!(x)) {printf("%s = %d (bad)\n", #x, x); ret = 1;} \
  else if (verbose) printf("%s = %d\n", #x, x);

#define ENV_FALSE(env) if (verbose) printf("  target %s is false\n", #env);
#define ENV_TRUE(env) { \
  ++numenvs;  \
  if (verbose) printf("  target %s is true\n", #env); \
}

/* Capture __is_target_environment() behavior before any includes. */
static int
get_target_environment(int verbose)
{
  int numenvs = 0;

#ifndef __is_target_environment
  if (verbose) printf("  __is_target_environment is undefined\n");
  (void) numenvs;
  return 0;
#else
  #if __is_target_environment(kernelkit)
    ENV_TRUE(kernelkit)
  #else
    ENV_FALSE(kernelkit)
  #endif
  #if __is_target_environment(macabi)
    ENV_TRUE(macabi)
  #else
    ENV_FALSE(macabi)
  #endif
  #if __is_target_environment(simulator)
    ENV_TRUE(simulator)
  #else
    ENV_FALSE(simulator)
  #endif
  #if __is_target_environment(xros)
    ENV_TRUE(xros)
  #else
    ENV_FALSE(xros)
  #endif
  #if __is_target_environment(exclavecore)
    ENV_TRUE(exclavecore)
  #else
    ENV_FALSE(exclavecore)
  #endif
  #if __is_target_environment(exclavekit)
    ENV_TRUE(exclavekit)
  #else
    ENV_FALSE(exclavekit)
  #endif
  return numenvs > 1;
#endif
}

/* Do this first, to test "bare include" case. */
#include <TargetConditionals.h>

#include <libgen.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
  int ret = 0, verbose = 0;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (get_target_environment(verbose)) {
    printf("  *** Multiple target environments enabled\n");
  }

  #ifdef TARGET_OS_MAC
  PRINT_VAR_NEEDNZ(TARGET_OS_MAC);
  #else
  PRINT_UNDEF(TARGET_OS_MAC);
  #endif

  #ifdef TARGET_OS_OSX
  PRINT_VAR_NEEDNZ(TARGET_OS_OSX);
  #else
  PRINT_UNDEF(TARGET_OS_OSX);
  #endif

  #ifdef TARGET_OS_IPHONE
  PRINT_VAR(TARGET_OS_IPHONE);
  #else
  PRINT_UNDEF(TARGET_OS_IPHONE);
  #endif

  #ifdef TARGET_OS_IOS
  PRINT_VAR(TARGET_OS_IOS);
  #else
  PRINT_UNDEF(TARGET_OS_IOS);
  #endif

  #ifdef TARGET_OS_WATCH
  PRINT_VAR(TARGET_OS_WATCH);
  #else
  PRINT_UNDEF(TARGET_OS_WATCH);
  #endif

  #ifdef TARGET_OS_TV
  PRINT_VAR(TARGET_OS_TV);
  #else
  PRINT_UNDEF(TARGET_OS_TV);
  #endif

  #ifdef TARGET_OS_SIMULATOR
  PRINT_VAR(TARGET_OS_SIMULATOR);
  #else
  PRINT_UNDEF(TARGET_OS_SIMULATOR);
  #endif

  #ifdef TARGET_OS_EMBEDDED
  PRINT_VAR(TARGET_OS_EMBEDDED);
  #else
  PRINT_UNDEF(TARGET_OS_EMBEDDED);
  #endif

  #ifdef TARGET_OS_RTKIT
  PRINT_VAR(TARGET_OS_RTKIT);
  #else
  PRINT_UNDEF(TARGET_OS_RTKIT);
  #endif

  #ifdef TARGET_OS_MACCATALYST
  PRINT_VAR(TARGET_OS_MACCATALYST);
  #else
  PRINT_UNDEF(TARGET_OS_MACCATALYST);
  #endif

  #ifdef TARGET_OS_VISION
  PRINT_VAR(TARGET_OS_VISION);
  #else
  PRINT_UNDEF(TARGET_OS_VISION);
  #endif

  #ifdef TARGET_OS_UIKITFORMAC
  PRINT_VAR(TARGET_OS_UIKITFORMAC);
  #else
  PRINT_UNDEF(TARGET_OS_UIKITFORMAC);
  #endif

  #ifdef TARGET_OS_DRIVERKIT
  PRINT_VAR(TARGET_OS_DRIVERKIT);
  #else
  PRINT_UNDEF(TARGET_OS_DRIVERKIT);
  #endif

  #ifdef TARGET_OS_WIN32
  PRINT_VAR(TARGET_OS_WIN32);
  #else
  PRINT_UNDEF(TARGET_OS_WIN32);
  #endif

  #ifdef TARGET_OS_WINDOWS
  PRINT_VAR(TARGET_OS_WINDOWS);
  #else
  PRINT_UNDEF(TARGET_OS_WINDOWS);
  #endif

  #ifdef TARGET_OS_LINUX
  PRINT_VAR(TARGET_OS_LINUX);
  #else
  PRINT_UNDEF(TARGET_OS_LINUX);
  #endif

  #ifdef TARGET_OS_UNIX
  PRINT_VAR(TARGET_OS_UNIX);
  #else
  PRINT_UNDEF(TARGET_OS_UNIX);
  #endif

  #ifdef DYNAMIC_TARGETS_ENABLED
  PRINT_VAR(DYNAMIC_TARGETS_ENABLED);
  #else
  PRINT_UNDEF(DYNAMIC_TARGETS_ENABLED);
  #endif

  #ifdef TARGET_CPU_PPC
  PRINT_VAR(TARGET_CPU_PPC);
  #else
  PRINT_UNDEF(TARGET_CPU_PPC);
  #endif

  #ifdef TARGET_CPU_PPC64
  PRINT_VAR(TARGET_CPU_PPC64);
  #else
  PRINT_UNDEF(TARGET_CPU_PPC64);
  #endif

  #ifdef TARGET_CPU_68K
  PRINT_VAR(TARGET_CPU_68K);
  #else
  PRINT_UNDEF(TARGET_CPU_68K);
  #endif

  #ifdef TARGET_CPU_X86
  PRINT_VAR(TARGET_CPU_X86);
  #else
  PRINT_UNDEF(TARGET_CPU_X86);
  #endif

  #ifdef TARGET_CPU_X86_64
  PRINT_VAR(TARGET_CPU_X86_64);
  #else
  PRINT_UNDEF(TARGET_CPU_X86_64);
  #endif

  #ifdef TARGET_CPU_ARM
  PRINT_VAR(TARGET_CPU_ARM);
  #else
  PRINT_UNDEF(TARGET_CPU_ARM);
  #endif

  #ifdef TARGET_CPU_ARM64
  PRINT_VAR(TARGET_CPU_ARM64);
  #else
  PRINT_UNDEF(TARGET_CPU_ARM64);
  #endif

  #ifdef TARGET_CPU_MIPS
  PRINT_VAR(TARGET_CPU_MIPS);
  #else
  PRINT_UNDEF(TARGET_CPU_MIPS);
  #endif

  #ifdef TARGET_CPU_SPARC
  PRINT_VAR(TARGET_CPU_SPARC);
  #else
  PRINT_UNDEF(TARGET_CPU_SPARC);
  #endif

  #ifdef TARGET_CPU_ALPHA
  PRINT_VAR(TARGET_CPU_ALPHA);
  #else
  PRINT_UNDEF(TARGET_CPU_ALPHA);
  #endif

  #ifdef TARGET_ABI_USES_IOS_VALUES
  PRINT_VAR(TARGET_ABI_USES_IOS_VALUES);
  #else
  PRINT_UNDEF(TARGET_ABI_USES_IOS_VALUES);
  #endif

  #ifdef TARGET_IPHONE_SIMULATOR
  PRINT_VAR(TARGET_IPHONE_SIMULATOR);
  #else
  PRINT_UNDEF(TARGET_IPHONE_SIMULATOR);
  #endif

  #ifdef TARGET_OS_NANO
  PRINT_VAR(TARGET_OS_NANO);
  #else
  PRINT_UNDEF(TARGET_OS_NANO);
  #endif

  printf("%s %s.\n", basename(argv[0]), ret ? "failed" : "passed");
  return ret;
}
