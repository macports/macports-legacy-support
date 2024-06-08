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
 * This is a manual "test" to report the values of a few macros related to
 * SDK selection.  It never "fails".
 */

/* Do this before everything else. */
#include <_macports_extras/sdkversion.h>

/* So we can delay including stdio.h */
int printf(const char *format, ...);

#define PRINT_VAR(x) printf("%s = %lld\n", #x, (long long) x)
#define PRINT_UNDEF(x) printf(#x " is undefined\n")

void
print_defs(void)
{
  #ifdef __APPLE__
  PRINT_VAR(__APPLE__);
  #else
  PRINT_UNDEF(__APPLE__);
  #endif
  #ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
  PRINT_VAR(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__);
  #else
  PRINT_UNDEF(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__);
  #endif

  printf("\n");

  #ifdef MAC_OS_X_VERSION_MIN_REQUIRED
  PRINT_VAR(MAC_OS_X_VERSION_MIN_REQUIRED);
  #else
  PRINT_UNDEF(MAC_OS_X_VERSION_MIN_REQUIRED);
  #endif
  #ifdef MAC_OS_X_VERSION_MAX_ALLOWED
  PRINT_VAR(MAC_OS_X_VERSION_MAX_ALLOWED);
  #else
  PRINT_UNDEF(MAC_OS_X_VERSION_MAX_ALLOWED);
  #endif
  #ifdef __MPLS_PRE_10_5_SDK
  PRINT_VAR(__MPLS_PRE_10_5_SDK);
  #else
  PRINT_UNDEF(__MPLS_PRE_10_5_SDK);
  #endif
  #ifdef __MPLS_PRE_10_10_SDK
  PRINT_VAR(__MPLS_PRE_10_10_SDK);
  #else
  PRINT_UNDEF(__MPLS_PRE_10_10_SDK);
  #endif
  #ifdef __MPLS_PRE_10_14_SDK
  PRINT_VAR(__MPLS_PRE_10_14_SDK);
  #else
  PRINT_UNDEF(__MPLS_PRE_10_14_SDK);
  #endif
  #ifdef __MPLS_PRE_14_0_SDK
  PRINT_VAR(__MPLS_PRE_14_0_SDK);
  #else
  PRINT_UNDEF(__MPLS_PRE_14_0_SDK);
  #endif
}

/* Do this afterward, since it might influence the definitions. */
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("\n");
  print_defs();
  printf("\n");

  return 0;
}
