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
 * This provides a limited test of *clonefile*(), mainly to test the disallowed
 * cases on HFS+, since native clonefile() support is available on all OS
 * versions that support APFS.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/attr.h>
#include <sys/clonefile.h>
#include <sys/fcntl.h>
#include <sys/mount.h>
#include <sys/param.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

/*
 * Determine filesystem cloning support for given file.
 *
 * -1 error, 0 no cloning allowed, 1 cloning allowed
 */
static int
get_cloneable(const char *path, int verbose)
{
  struct statfs sfb;
  struct attrlist al = {
      .bitmapcount = ATTR_BIT_MAP_COUNT,
      .volattr = ATTR_VOL_INFO | ATTR_VOL_CAPABILITIES,
      };
  uint32_t abuf[1024] = {0};
  vol_capabilities_attr_t *vc;

  if (statfs(path, &sfb)) {
    printf("  statfs() for '%s' failed: %s\n", path, strerror(errno));
    return -1;
  }
  if (verbose) printf("  %s is in volume %s\n", path, sfb.f_mntonname);

  if (getattrlist(sfb.f_mntonname, &al, abuf, sizeof(abuf), 0)) {
    fprintf(stderr, "  getattrlist() for '%s' failed: %s\n",
            sfb.f_mntonname, strerror(errno));
    return -1;
  }
  if (abuf[0] != sizeof(*vc) + sizeof(abuf[0])) {
    printf("  getattrlist() for '%s' returned wrong size: %d\n",
           sfb.f_mntonname, abuf[0]);
    return -1;
  }

  vc = (vol_capabilities_attr_t *) &abuf[1];
  if (vc->valid[VOL_CAPABILITIES_INTERFACES] & VOL_CAP_INT_CLONE) {
    if (vc->capabilities[VOL_CAPABILITIES_INTERFACES] & VOL_CAP_INT_CLONE) {
      if (verbose) printf("    VOL_CAP_INT_CLONE is set\n");
      return 1;
    } else {
      if (verbose) printf("    VOL_CAP_INT_CLONE is not set\n");
      return 0;
    }
  } else {
    if (verbose) printf("    VOL_CAP_INT_CLONE is unspecified\n");
    return 0;
  }
}

static int
check_status(int status, const char *call, int verbose)
{
  if (status != -1) {
    printf("  %s unexpectedly succeeded\n", call);
    return 1;
  }
  if (errno != ENOTSUP) {
    printf("  %s returned incorrect errno: %s (%d)\n",
           call, strerror(errno), errno);
    return 1;
  }
  if (verbose) {
    printf("  %s returned expected errno: %s (%d)\n",
           call, strerror(errno), errno);
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, cloneable, fd;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  char dest[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(dest, sizeof(dest), "%s/%s-%u", TEST_TEMP, progname, pid);

  if (verbose) {
    printf("%s starting.\n", progname);
    printf("  %s -> %s\n", argv[0], dest);
  }

  cloneable = get_cloneable(argv[0], verbose);
  if (cloneable < 0) {
    printf("%s failed.\n", progname);
    return 1;
  }
  if (cloneable) {
    printf("%s skipped due to cloneable volume.\n", progname);
    return 0;
  }

  ret |= check_status(
      clonefile(argv[0], dest, 0),
      "clonefile(<src>, <dst>, 0)",
      verbose
      );
  ret |= check_status(
      clonefileat(AT_FDCWD, argv[0], AT_FDCWD, dest, 0),
      "clonefileat(AT_FDCWD, <src>, AT_FDCWD, <dst>, 0)",
      verbose
      );
  if ((fd = open(argv[0], O_RDONLY)) < 0) {
    printf("  open() for %s failed: %s\n", argv[0], strerror(errno));
    ret = 1;
  } else {
    ret |= check_status(
        fclonefileat(fd, AT_FDCWD, dest, 0),
        "fclonefileat(<srcfd>, AT_FDCWD, <dst>, 0)",
        verbose
        );
    (void) close(fd);
  }

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
