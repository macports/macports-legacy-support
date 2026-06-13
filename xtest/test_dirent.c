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
 * An earlier implementation of fdopendir() defined seekdir as a macro,
 * which originally conflicted with a C++ library definition.  It was fixed
 * to avoid that, and the test that this replaces was written in C++ in
 * order to verify the fix.  But C++ tests have become more problematic
 * as of the 15.x SDK, and meanwhile, the fdopendir() implementation was
 * rewritten to use a different approach, making the old test moot.
 *
 * This test was originally just a dummy to verify the include of dirent.h.
 * The original C++ test still exists, but now as:
 *   manual_tests/dirent_with_cplusplus.cpp
 *
 * This test has now been expanded to test the use of the double-underscore
 * versions of the DIR element names, which normally don't exist in 10.4,
 * but are now provided by our wrapper.  In 10.4, it verifies that both
 * versions of each name match; otherwise it just verifies that the usual
 * double-underscore names can be referenced.
 */

#include <dirent.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

static DIR test;

#define DIRENTRIES \
  DIRITEM(dd_fd) \
  DIRITEM(dd_loc) \
  DIRITEM(dd_size) \
  DIRITEM(dd_buf) \
  DIRITEM(dd_len) \
  DIRITEM(dd_seek) \
  /* DIRITEM(dd_rewind) */ \
  DIRITEM(dd_flags) \
  DIRITEM(dd_lock) \
  DIRITEM(dd_td) \

/* The __dd_rewind item disappeared as of 10.13 */
#if __MPLS_SDK_MAJOR < 101300
  #define XDIRENTRIES \
    DIRITEM(dd_rewind)
#else  /* 10.13+ */
  #define XDIRENTRIES
#endif  /* 10.13+ */

#if __MPLS_SDK_MAJOR >= 1050
  #define DIRITEM(name) (void) test.__ ## name;
#else  /* 10.4 SDK */
  #define DIRITEM(name) \
    if (&test.name != &test.__ ## name) { \
      printf("  *** DIR name '%s' mismatches\n", # name); \
      err = 1; \
    }
#endif  /* 10.4 SDK */

int
main(int argc, char *argv[])
{
  int err = 0;

  (void) argc; (void) argv;

  DIRENTRIES
  XDIRENTRIES

  printf("%s %s.\n", basename(argv[0]), err ? "failed" : "passed");
  return err;
}
