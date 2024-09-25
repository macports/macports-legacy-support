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
 * This tests type-related issues with scandir().  It does not perform
 * any functional tests, since the real function is never provided by
 * legacy-support.
 *
 * Any errors are in the form of compiler warnings about type mismatches.
 * These only cause actual errors with -Werror or a picky compiler.
 *
 * The nominal runtime test here is just a dummy which always succeeds.
 */

/* Get the SDK version, since it's relevant to the issue being tested. */
#include <_macports_extras/sdkversion.h>

/* Enable the compatibility wrapper by default. */
#ifndef _MACPORTS_LEGACY_COMPATIBLE_SCANDIR
#define _MACPORTS_LEGACY_COMPATIBLE_SCANDIR 1
#endif

#include <dirent.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>

/* Determine whether the old or new type of the 'compar' func is in effect. */
#if !_MACPORTS_LEGACY_COMPATIBLE_SCANDIR && __MPLS_SDK_MAJOR < 1080
#define OLD_COMPAR 1
#else
#define OLD_COMPAR 0
#endif

/* Dummy 'compar' function of expected type */
#if OLD_COMPAR
static int
mysort(const void *d1, const void *d2)
{
  (void) d1; (void) d2;
  return 0;
}
#else /* !OLD_COMPAR */
static int
mysort(const struct dirent **d1, const struct dirent **d2)
{
  (void) d1; (void) d2;
  return 0;
}
#endif /* !OLD_COMPAR */

/*
 * Function to theoretically call scandir() (not actually invoked).
 * This is extern to avoid being optimized out, and tests the return
 * values to avoid the calls being optimized out.
 */
int test_scandir(void);  /* Avoid missing-prototype warning. */

int
test_scandir(void)
{
  struct dirent **names;
  if (scandir(".", &names, NULL, alphasort)) return -1;
  if (scandir(".", &names, NULL, mysort)) return -1;
  return 0;
}

/* Also make sure we can take pointers to the possibly wrapped functions. */
void *scandir_p = &scandir;
void *alphasort_p = &alphasort;

/* Dummy runtime test */
int
main(int argc, char *argv[])
{
  (void) argc;
  printf("%s passed.\n", basename(argv[0]));
  return 0;
}
