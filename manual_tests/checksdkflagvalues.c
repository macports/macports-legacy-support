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
 * This is a manual test to check the SDK flag values against the SDK version
 * supplied the SDKVER environment variable (defaulting to the target OS
 * version), in the same format as MacOSX<version>.sdk.
 */

/* Do this before everything else. */
#include <_macports_extras/sdkversion.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define TARGET_OS __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define TARGET_OS 1040
#endif

typedef struct varinfo_s {
  const char *name;
  int val;
  int thresh;
} varinfo_t;

/*
 * Table of observed flag values and expected thresholds.
 * All possible flag names (+1) are included, whether implemented or not.
 * Expand this list when new SDKs are issued.
 */
const varinfo_t val_map[] = {
  {
    .name = "__MPLS_PRE_10_5_SDK",
    .val =
    #ifndef __MPLS_PRE_10_5_SDK
    -1,
    #else
    __MPLS_PRE_10_5_SDK,
    #endif
    .thresh = 1050,
  },
  {
    .name = "__MPLS_PRE_10_6_SDK",
    .val =
    #ifndef __MPLS_PRE_10_6_SDK
    -1,
    #else
    __MPLS_PRE_10_6_SDK,
    #endif
    .thresh = 1060,
  },
  {
    .name = "__MPLS_PRE_10_7_SDK",
    .val =
    #ifndef __MPLS_PRE_10_7_SDK
    -1,
    #else
    __MPLS_PRE_10_7_SDK,
    #endif
    .thresh = 1070,
  },
  {
    .name = "__MPLS_PRE_10_8_SDK",
    .val =
    #ifndef __MPLS_PRE_10_8_SDK
    -1,
    #else
    __MPLS_PRE_10_8_SDK,
    #endif
    .thresh = 1080,
  },
  {
    .name = "__MPLS_PRE_10_9_SDK",
    .val =
    #ifndef __MPLS_PRE_10_9_SDK
    -1,
    #else
    __MPLS_PRE_10_9_SDK,
    #endif
    .thresh = 1090,
  },
  {
    .name = "__MPLS_PRE_10_10_SDK",
    .val =
    #ifndef __MPLS_PRE_10_10_SDK
    -1,
    #else
    __MPLS_PRE_10_10_SDK,
    #endif
    .thresh = 101000,
  },
  {
    .name = "__MPLS_PRE_10_11_SDK",
    .val =
    #ifndef __MPLS_PRE_10_11_SDK
    -1,
    #else
    __MPLS_PRE_10_11_SDK,
    #endif
    .thresh = 101100,
  },
  {
    .name = "__MPLS_PRE_10_12_SDK",
    .val =
    #ifndef __MPLS_PRE_10_12_SDK
    -1,
    #else
    __MPLS_PRE_10_12_SDK,
    #endif
    .thresh = 101200,
  },
  {
    .name = "__MPLS_PRE_10_13_SDK",
    .val =
    #ifndef __MPLS_PRE_10_13_SDK
    -1,
    #else
    __MPLS_PRE_10_13_SDK,
    #endif
    .thresh = 101300,
  },
  {
    .name = "__MPLS_PRE_10_14_SDK",
    .val =
    #ifndef __MPLS_PRE_10_14_SDK
    -1,
    #else
    __MPLS_PRE_10_14_SDK,
    #endif
    .thresh = 101400,
  },
  {
    .name = "__MPLS_PRE_10_15_SDK",
    .val =
    #ifndef __MPLS_PRE_10_15_SDK
    -1,
    #else
    __MPLS_PRE_10_15_SDK,
    #endif
    .thresh = 101500,
  },
  {
    .name = "__MPLS_PRE_11_0_SDK",
    .val =
    #ifndef __MPLS_PRE_11_0_SDK
    -1,
    #else
    __MPLS_PRE_11_0_SDK,
    #endif
    .thresh = 110000,
  },
  {
    .name = "__MPLS_PRE_12_0_SDK",
    .val =
    #ifndef __MPLS_PRE_12_0_SDK
    -1,
    #else
    __MPLS_PRE_12_0_SDK,
    #endif
    .thresh = 120000,
  },
  {
    .name = "__MPLS_PRE_13_0_SDK",
    .val =
    #ifndef __MPLS_PRE_13_0_SDK
    -1,
    #else
    __MPLS_PRE_13_0_SDK,
    #endif
    .thresh = 130000,
  },
  {
    .name = "__MPLS_PRE_14_0_SDK",
    .val =
    #ifndef __MPLS_PRE_14_0_SDK
    -1,
    #else
    __MPLS_PRE_14_0_SDK,
    #endif
    .thresh = 140000,
  },
  {
    .name = "__MPLS_PRE_15_0_SDK",
    .val =
    #ifndef __MPLS_PRE_15_0_SDK
    -1,
    #else
    __MPLS_PRE_15_0_SDK,
    #endif
    .thresh = 150000,
  },
};
#define VAL_MAP_SIZE (sizeof(val_map) / sizeof(val_map[0]))

int
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
  if (major == 10 && minor <= 9) return major * 100 + minor * 10;
  return major * 10000 + minor * 100;
}

int
main(int argc, char *argv[])
{
  const char *sdkver = NULL;
  int sdknum, correct, ret = 0, verbose = 0;
  const varinfo_t *vp;

  if (argc > 1 && strcmp(argv[1], "-v") == 0) verbose = 1;

  sdkver = getenv("SDKVER");
  sdknum = get_sdknum(sdkver);
  if (sdknum < 0) {
    fprintf(stderr, "Bad SDK version: %s\n", sdkver ? sdkver : "???");
    return 20;
  }

  printf("Testing SDK version %s,%s numeric = %d\n",
         sdkver ? sdkver : "<default>", sdkver ? "" : " assumed", sdknum);

  for (vp = &val_map[0]; vp < &val_map[VAL_MAP_SIZE]; ++vp) {
    if (vp->val < 0) {
      if (verbose) printf("  %s is undefined\n", vp->name);
      continue;
    }
    correct = sdknum < vp->thresh;
    if (vp->val != correct) {
      printf("  %s is incorrectly %d, should be %d\n",
             vp->name, vp->val, correct);
      ret = 1;
    } else {
      if (verbose) printf("  %s is correctly %d\n", vp->name, vp->val);
    }
  }
  if (!ret) printf("All defined values are correct\n");
  return ret;
}
