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

/*
 * This provides tests of mkostemp[s]().  It also indirectly tests the
 * support for O_CLOEXEC in open() on <10.7.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define PREFIX TEST_TEMP "/temp."
#define FILL "XXXXXXXX"
#define SUFFIX ".ext"

#define TEST_NOSFX PREFIX FILL
#define TEST_SFX TEST_NOSFX SUFFIX

#define LEN_PFX (sizeof(PREFIX) - 1)
#define LEN_FILL (sizeof(FILL) - 1)
#define LEN_SFX (sizeof(SUFFIX) -1)

#define CASES \
  CASE(0) \
  CASE(-1) \
  CASE(O_APPEND) \
  CASE(O_SHLOCK) \
  CASE(O_EXLOCK) \
  CASE(O_CLOEXEC) \
  CASE(O_APPEND|O_SHLOCK|O_CLOEXEC) \
  CASE(O_APPEND|O_EXLOCK|O_CLOEXEC) \

typedef struct case_s {
  int flags;
  const char *name;
} case_t;

#define CASE(x) {x, #x},
case_t cases[] = {
  CASES
};
#undef CASE
#define NUM_CASES (sizeof(cases) / sizeof(cases[0]))

static int
test_case(int withsfx, case_t *test, int verbose)
{
  int fd, ret = 0, flags, cloexec;
  size_t len;
  char path[PATH_MAX];

  strcpy(path, withsfx ? TEST_SFX : TEST_NOSFX);
  len = strlen(path);
  if (verbose) printf("  testing %s with %s\n", path, test->name);

  fd = withsfx ? mkostemps(path, LEN_SFX, test->flags)
               : mkostemp(path, test->flags);
  if (fd < 0) {
    if (test->flags == -1 && errno == EINVAL) {
      if (verbose) printf("    correctly failed with invalid argument\n");
      return 0;
    }
    printf("  *** mkostemp%s() failed: %s\n",
           withsfx ? "s" : "", strerror(errno));
    return 1;
  } else if (test->flags == -1) {
    printf("  *** incorrectly succeeded with invalid argument\n");
    ret = 1;
  }

  if (strlen(path) != len) ret = 1;
  if (strncmp(path, PREFIX, LEN_PFX)) ret = 1;
  if (!strncmp(path + LEN_PFX, FILL, LEN_FILL)) ret = 1;
  if (withsfx && strncmp(path + LEN_PFX + LEN_FILL, SUFFIX, LEN_SFX)) ret = 1;
  
  if (ret || verbose) {
    printf("  %sresulting path is '%s'\n", ret ? "*** " : "  ", path);
  }

  do {
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
      printf("  *** fcntl(F_GETFL) failed: %s\n", strerror(errno));
      ret = 1;
      break;
    }
    cloexec = fcntl(fd, F_GETFD, 0);
    if (cloexec == -1) {
      printf("  *** fcntl(F_GETFD) failed: %s\n", strerror(errno));
      ret = 1;
      break;
    }
    if (cloexec & 1) flags |= O_CLOEXEC; else flags &= ~O_CLOEXEC;

    if ((flags ^ test->flags) & (O_APPEND | O_CLOEXEC)) {
      printf("  *** resulting flags 0x%X mismatched requested 0x%X\n",
             flags, test->flags);
      ret = 1;
    }
  } while (0);

  (void) close(fd);
  (void) unlink(path);
  return ret;
}

static int
test_two(case_t *test, int verbose)
{
  int ret = 0;

  ret |= test_case(0, test, verbose);
  ret |= test_case(1, test, verbose);
  return ret;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, i;
  char *progname = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", progname);

  for (i = 0; i < NUM_CASES; ++i) {
    ret |= test_two(&cases[i], verbose);
  }
  
  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
