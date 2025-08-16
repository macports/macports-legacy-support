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
 * This tests some of the functions of fcntl().  Unfortunately, due to the
 * variadic nature of fcntl() and the lack of a corresponding vfcntl(), our
 * wrapper is forced to handle all possible commands just to get the correct
 * type for the third argument.  Including all commands in this test would
 * be massive, and some can't be tested without additional restrictions,
 * so we simply try to cover as many argument types as possible.  We can't
 * easily test F_SETOWN or F_SETSIZE, but we can cover the none, int, and
 * pointer cases of the argument type.
 *
 * The only function of fcntl() that we actually modify in the library is
 * F_GETPATH, and that one only for 10.4 ppc64.  It's already implicitly
 * tested via the fxattrlist tests, but we also test it here, which also
 * provides coverage for the "pointer" arg-type case.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, fd = -1, val;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  char tpath[MAXPATHLEN];
  char rpath[MAXPATHLEN], xpath[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(tpath, sizeof(tpath), "%s/%s-%u", TEST_TEMP, progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  do {
    if (verbose) printf("  creating '%s'\n", tpath);
    if ((fd = open(tpath, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
       printf("*** unable to open '%s': %s\n", tpath, strerror(errno));
       ret = 1;
       break;
    }

    if (!realpath(tpath, rpath)) {
      printf("  *** realpath() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret =1;
      break;
    }

    if (verbose) printf("  testing F_SETFD(FD_CLOEXEC)\n");
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
      printf("  *** fcntl(fd, F_SETFD, FD_CLOEXEC) failed: %s\n",
             strerror(errno));
      ret = 1;
    }

    if (verbose) printf("  testing F_GETFD\n");
    if ((val = fcntl(fd, F_GETFD)) == -1) {
      printf("  *** fcntl(fd, F_GETFD) failed: %s\n",
             strerror(errno));
      ret = 1;
    } else if (val != FD_CLOEXEC) {
      printf("  *** F_GETFD returned %d, should be %d\n",
             val, FD_CLOEXEC);
      ret = 1;
    }

    if (verbose) printf("  testing F_GETPATH\n");
    if ((val = fcntl(fd, F_GETPATH, xpath)) == -1) {
      printf("  *** fcntl(fd, F_GETPATH, xpath) failed: %s\n",
             strerror(errno));
      ret = 1;
    } else {
      xpath[MAXPATHLEN-1] = '\0';
      if (strcmp(xpath, rpath)) {
        printf("  *** F_GETPATH returned %s, should be %s\n",
               xpath, rpath);
        ret = 1;
      }
    }
  } while (0);

  if (fd >= 0) (void) close(fd);
  (void) unlink(tpath);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
