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
 * This is a manual test to check __MPLS_SDK_MAJOR against the SDK version
 * supplied via the SDKVER environment variable (defaulting to the target OS
 * version), in the same format as MacOSX<version>.sdk.
 */

/* Do this before everything else. */
#include <_macports_extras/sdkversion.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define TARGET_OS __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define TARGET_OS 1040
#endif

static int
get_sdknum(const char *sdkver)
{
  long major, minor;
  char *endp;

  if (!sdkver || !*sdkver) return TARGET_OS;

  major = strtol(sdkver, &endp, 10);
  if (*endp == '.') {
    minor = strtol(endp + 1, &endp, 10);
  } else {
    minor = 0;
  }
  if (major < 10 || (major > 10 && minor != 0)) return -1;
  if (*endp && (major != 10 || minor != 4 || *endp != 'u')) return -1;
  if (major == 10 && minor <= 9) return (int) (major * 100 + minor * 10);
  return (int) (major * 10000 + minor * 100);
}

int
main(int argc, char *argv[])
{
  const char *sdkver = NULL;
  int sdknum, sdkmajor;

  (void) argc; (void) argv;

  sdkver = getenv("SDKVER");
  sdknum = get_sdknum(sdkver);
  if (sdknum < 0) {
    fprintf(stderr, "Bad SDK version: %s\n", sdkver ? sdkver : "???");
    return 20;
  }
  sdkmajor = sdknum / 10 * 10;

  printf("Testing SDK version %s,%s numeric = %d, major = %d\n",
         sdkver ? sdkver : "<default>", sdkver ? "" : " assumed",
         sdknum, sdkmajor);

  #ifndef __MPLS_SDK_MAJOR
    printf("  __MPLS_SDK_MAJOR is undefined\n");
    return 1;
  #else
    if (__MPLS_SDK_MAJOR != sdkmajor) {
      printf("  __MPLS_SDK_MAJOR is %d, should be %d\n",
             __MPLS_SDK_MAJOR, sdkmajor);
      return 2;
    } else {
      printf("  __MPLS_SDK_MAJOR is correctly %d\n", sdkmajor);
    }
  #endif

  return 0;
}
