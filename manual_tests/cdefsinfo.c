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
#define PRINT_SVAR(x) printf("%s = \"%s\"\n", #x, "" x)
#define PRINT_UNDEF(x) printf(#x " is undefined\n")

static void
print_compiler(void)
{
  #ifdef __cplusplus
  PRINT_VAR(__cplusplus);
  #else
  PRINT_UNDEF(__cplusplus);
  #endif

  #ifdef __GNUC__
  PRINT_VAR(__GNUC__);
  #else
  PRINT_UNDEF(__GNUC__);
  #endif
  #ifdef __GNUC_MINOR__
  PRINT_VAR(__GNUC_MINOR__);
  #else
  PRINT_UNDEF(__GNUC_MINOR__);
  #endif

  #ifdef __clang_major__
  PRINT_VAR(__clang_major__);
  #else
  PRINT_UNDEF(__clang_major__);
  #endif
  #ifdef __clang_minor__
  PRINT_VAR(__clang_minor__);
  #else
  PRINT_UNDEF(__clang_minor__);
  #endif

  #ifdef __APPLE_CC__
  PRINT_VAR(__APPLE_CC__);
  #else
  PRINT_UNDEF(__APPLE_CC__);
  #endif
  #ifdef __apple_build_version__
  PRINT_VAR(__apple_build_version__);
  #else
  PRINT_UNDEF(__apple_build_version__);
  #endif
}

static void
print_before_cdefs(void)
{
  printf("  Before <sys/cdefs.h>:\n");

  #ifdef _POSIX_C_SOURCE
  PRINT_VAR(_POSIX_C_SOURCE);
  #else
  PRINT_UNDEF(_POSIX_C_SOURCE);
  #endif
  #ifdef _XOPEN_SOURCE
  PRINT_VAR(_XOPEN_SOURCE);
  #else
  PRINT_UNDEF(_XOPEN_SOURCE);
  #endif
  #ifdef _ANSI_SOURCE
  PRINT_VAR(_ANSI_SOURCE);
  #else
  PRINT_UNDEF(_ANSI_SOURCE);
  #endif
  #ifdef _DARWIN_C_SOURCE
  PRINT_VAR(_DARWIN_C_SOURCE);
  #else
  PRINT_UNDEF(_DARWIN_C_SOURCE);
  #endif
  #ifdef _NONSTD_SOURCE
  PRINT_VAR(_NONSTD_SOURCE);
  #else
  PRINT_UNDEF(_NONSTD_SOURCE);
  #endif
  #ifdef __DARWIN_C_ANSI
  PRINT_VAR(__DARWIN_C_ANSI);
  #else
  PRINT_UNDEF(__DARWIN_C_ANSI);
  #endif
  #ifdef __DARWIN_C_FULL
  PRINT_VAR(__DARWIN_C_FULL);
  #else
  PRINT_UNDEF(__DARWIN_C_FULL);
  #endif
  #ifdef __DARWIN_C_LEVEL
  PRINT_VAR(__DARWIN_C_LEVEL);
  #else
  PRINT_UNDEF(__DARWIN_C_LEVEL);
  #endif
  #ifdef __LP64__
  PRINT_VAR(__LP64__);
  #else
  PRINT_UNDEF(__LP64__);
  #endif
  #ifdef __DARWIN_ONLY_UNIX_CONFORMANCE
  PRINT_VAR(__DARWIN_ONLY_UNIX_CONFORMANCE);
  #else
  PRINT_UNDEF(__DARWIN_ONLY_UNIX_CONFORMANCE);
  #endif
  #ifdef __DARWIN_UNIX03
  PRINT_VAR(__DARWIN_UNIX03);
  #else
  PRINT_UNDEF(__DARWIN_UNIX03);
  #endif
  #ifdef __DARWIN_SUF_UNIX03
  PRINT_SVAR(__DARWIN_SUF_UNIX03);
  #else
  PRINT_UNDEF(__DARWIN_SUF_UNIX03);
  #endif
  #ifdef _DARWIN_NO_64_BIT_INODE
  PRINT_VAR(_DARWIN_NO_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_NO_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_USE_64_BIT_INODE
  PRINT_VAR(_DARWIN_USE_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_USE_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_FEATURE_64_BIT_INODE
  PRINT_VAR(_DARWIN_FEATURE_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_FEATURE_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_FEATURE_ONLY_64_BIT_INODE
  PRINT_VAR(_DARWIN_FEATURE_ONLY_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_FEATURE_ONLY_64_BIT_INODE);
  #endif
  #ifdef __DARWIN_64_BIT_INO_T
  PRINT_VAR(__DARWIN_64_BIT_INO_T);
  #else
  PRINT_UNDEF(__DARWIN_64_BIT_INO_T);
  #endif
  #ifdef __DARWIN_ONLY_64_BIT_INODE
  PRINT_VAR(__DARWIN_ONLY_64_BIT_INODE);
  #else
  PRINT_UNDEF(__DARWIN_ONLY_64_BIT_INODE);
  #endif
  #ifdef __DARWIN_SUF_64_BIT_INO_T
  PRINT_SVAR(__DARWIN_SUF_64_BIT_INO_T);
  #else
  PRINT_UNDEF(__DARWIN_SUF_64_BIT_INO_T);
  #endif
}

#include <sys/cdefs.h>

static void
print_after_cdefs(void)
{
  printf("  After <sys/cdefs.h>:\n");

  #ifdef _POSIX_C_SOURCE
  PRINT_VAR(_POSIX_C_SOURCE);
  #else
  PRINT_UNDEF(_POSIX_C_SOURCE);
  #endif
  #ifdef _XOPEN_SOURCE
  PRINT_VAR(_XOPEN_SOURCE);
  #else
  PRINT_UNDEF(_XOPEN_SOURCE);
  #endif
  #ifdef _ANSI_SOURCE
  PRINT_VAR(_ANSI_SOURCE);
  #else
  PRINT_UNDEF(_ANSI_SOURCE);
  #endif
  #ifdef _DARWIN_C_SOURCE
  PRINT_VAR(_DARWIN_C_SOURCE);
  #else
  PRINT_UNDEF(_DARWIN_C_SOURCE);
  #endif
  #ifdef _NONSTD_SOURCE
  PRINT_VAR(_NONSTD_SOURCE);
  #else
  PRINT_UNDEF(_NONSTD_SOURCE);
  #endif
  #ifdef __DARWIN_C_ANSI
  PRINT_VAR(__DARWIN_C_ANSI);
  #else
  PRINT_UNDEF(__DARWIN_C_ANSI);
  #endif
  #ifdef __DARWIN_C_FULL
  PRINT_VAR(__DARWIN_C_FULL);
  #else
  PRINT_UNDEF(__DARWIN_C_FULL);
  #endif
  #ifdef __DARWIN_C_LEVEL
  PRINT_VAR(__DARWIN_C_LEVEL);
  #else
  PRINT_UNDEF(__DARWIN_C_LEVEL);
  #endif
  #ifdef __LP64__
  PRINT_VAR(__LP64__);
  #else
  PRINT_UNDEF(__LP64__);
  #endif
  #ifdef __DARWIN_ONLY_UNIX_CONFORMANCE
  PRINT_VAR(__DARWIN_ONLY_UNIX_CONFORMANCE);
  #else
  PRINT_UNDEF(__DARWIN_ONLY_UNIX_CONFORMANCE);
  #endif
  #ifdef __DARWIN_UNIX03
  PRINT_VAR(__DARWIN_UNIX03);
  #else
  PRINT_UNDEF(__DARWIN_UNIX03);
  #endif
  #ifdef __DARWIN_SUF_UNIX03
  PRINT_SVAR(__DARWIN_SUF_UNIX03);
  #else
  PRINT_UNDEF(__DARWIN_SUF_UNIX03);
  #endif
  #ifdef _DARWIN_NO_64_BIT_INODE
  PRINT_VAR(_DARWIN_NO_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_NO_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_USE_64_BIT_INODE
  PRINT_VAR(_DARWIN_USE_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_USE_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_FEATURE_64_BIT_INODE
  PRINT_VAR(_DARWIN_FEATURE_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_FEATURE_64_BIT_INODE);
  #endif
  #ifdef _DARWIN_FEATURE_ONLY_64_BIT_INODE
  PRINT_VAR(_DARWIN_FEATURE_ONLY_64_BIT_INODE);
  #else
  PRINT_UNDEF(_DARWIN_FEATURE_ONLY_64_BIT_INODE);
  #endif
  #ifdef __DARWIN_64_BIT_INO_T
  PRINT_VAR(__DARWIN_64_BIT_INO_T);
  #else
  PRINT_UNDEF(__DARWIN_64_BIT_INO_T);
  #endif
  #ifdef __DARWIN_ONLY_64_BIT_INODE
  PRINT_VAR(__DARWIN_ONLY_64_BIT_INODE);
  #else
  PRINT_UNDEF(__DARWIN_ONLY_64_BIT_INODE);
  #endif
  #ifdef __DARWIN_SUF_64_BIT_INO_T
  PRINT_SVAR(__DARWIN_SUF_64_BIT_INO_T);
  #else
  PRINT_UNDEF(__DARWIN_SUF_64_BIT_INO_T);
  #endif
}

/* Include stdio afterward, since it might influence the definitions. */
/* Disable the PPC long double hack to avoid a warning. */
#undef __DARWIN_LDBL_COMPAT
#define __DARWIN_LDBL_COMPAT(x)
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("\n");
  print_compiler();
  printf("\n");
  PRINT_VAR(__MPLS_SDK_MAJOR);
  printf("\n");
  print_before_cdefs();
  printf("\n");
  print_after_cdefs();
  printf("\n");

  return 0;
}
