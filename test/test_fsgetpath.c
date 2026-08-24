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

/* This provides a basic test of fsgetpath(). */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/attr.h>
#include <sys/fsgetpath.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <_macports_extras/targetos.h>

#ifdef __LP64__
typedef unsigned int attrlist_opts_t;
#else /* !__LP64__ */
typedef unsigned long attrlist_opts_t;
#endif /* !__LP64__ */

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define ULL (unsigned long long)

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0;
  ssize_t plen, actlen;
  char *progname = basename(argv[0]);
  struct statfs sfs = {0};
  struct stat sb;
  char path[PATH_MAX];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", progname);

  do {

    if (verbose) printf("  testing '" TEST_TEMP "'\n");

    if (statfs(TEST_TEMP, &sfs)) {
      printf("  *** statfs() for '" TEST_TEMP "' failed: %s (%d)\n",
             strerror(errno), errno);
      ret = 1;
      break;
    } else {
      if (verbose) printf("    filesystem type is '%s'\n", sfs.f_fstypename);
    }

    if (stat(TEST_TEMP, &sb)) {
      printf("  *** stat() for '" TEST_TEMP "' failed: %s (%d)\n",
             strerror(errno), errno);
      ret = 1;
      break;
    } else {
      if (verbose) printf("    inode is %llu\n", ULL sb.st_ino);
    }

    plen = fsgetpath(path, sizeof(path), &sfs.f_fsid, sb.st_ino);
    if (plen < 0) {
      if (errno == ENOTSUP && __MPLS_TARGET_OSVER < 1060) {
        if (verbose) {
          printf("  --- tolerating unsupported fsgetpath() on < 10.6\n");
        }
      } else {
        printf("  *** fsgetpath() failed: %s (%d)\n", strerror(errno), errno);
        ret = 1;
      }
      break;
    }

    actlen = strnlen(path, sizeof(path));
    if (plen != actlen + 1) {
      printf("  *** returned length %zd mismatched actual %zd+!\n",
             plen, actlen);
      ret = 1;
      break;
    }

    if (verbose) printf("    returned path is '%s'\n", path);

  } while (0);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
