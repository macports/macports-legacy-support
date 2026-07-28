/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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

/* This is a simple test of the aligned allocation functions. */

/* Override language-version condition */
#define _MACPORTS_LEGACY_ALLOW_ALIGNED_ALLOC 1

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALIGNMENT       256
#define LARGE_SIZE      512
#define SMALL_SIZE       64
#define UNLALIGNED_SIZE 101

#ifdef __LP64__
typedef unsigned long long ptrint_t;
#define PTR_FMT "%016llX"
#else
typedef unsigned int ptrint_t;
#define PTR_FMT "%08X"
#endif

static void *
do_alloc(int align, int size, int posix)
{
  void *result;

  if (!align) {
    result = malloc(size);
  } else if (posix) {
    if (posix_memalign(&result, align, size)) result = NULL;
  } else {
    result = aligned_alloc(align, size);
  }
  if (!result) {
    perror("  *** alloc failed");
    return NULL;
  }
  if (align && (ptrint_t) result & (align - 1)) {
    fprintf(stderr, "  *** adr 0x" PTR_FMT " isn't aligned to %d\n",
            (ptrint_t) result, align);
    free(result);
    return NULL;
  }
  return result;
}

static int
do_test(int size, int posix, int verbose)
{
  int ret = 1;
  void *align1 = NULL, *noalign = NULL, *align2 = NULL;

  if (verbose) printf("  testing %s() for size %d\n",
                      posix ? "posix_memalign" : "aligned_alloc", size);

  do {
    if (!(align1 = do_alloc(ALIGNMENT, size, posix))) break;
    if (!(noalign = do_alloc(0, UNLALIGNED_SIZE, posix))) break;
    if (!(align2 = do_alloc(ALIGNMENT, size, posix))) break;
    ret = 0;
    if (verbose) {
      printf("    got 0x" PTR_FMT ", 0x" PTR_FMT ", 0x" PTR_FMT "\n",
             (ptrint_t) align1, (ptrint_t) noalign, (ptrint_t) align2);
    }
  } while (0);

  free(align2);
  free(noalign);
  free(align1);
  return ret;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0;
  char *progname = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("Starting %s\n", progname);

  ret |= do_test(SMALL_SIZE, 1, verbose);
  ret |= do_test(LARGE_SIZE, 1, verbose);
  ret |= do_test(LARGE_SIZE, 0, verbose);

  printf("%s %s.\n", progname, ret ? "failed" : "succeeded");
  return ret;
}
