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
 * This provides some basic tests of "at" functions that aren't covered by
 * other tests.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define MAX_GROUPS 200

typedef struct paths_s {
  int isdir;
  char top[PATH_MAX];
  char rel[PATH_MAX];
  char abs[PATH_MAX];
} paths_t;

static paths_t paths[4];
#define NUM_PATHS (sizeof(paths) / sizeof(paths[0]))

static void
setpaths(paths_t *pths, const char *progname, pid_t pid,
         int num, paths_t *parent)
{
  char *parentdir = TEST_TEMP;
  char testabs[PATH_MAX];

  if (parent) parentdir = parent->top;
  pths->isdir = 0;
  (void) snprintf(pths->top, PATH_MAX,
                  "%s/%s-%u-%d", parentdir, progname, pid, num);
  (void) snprintf(pths->rel, PATH_MAX,
                  "%s-%u-%d", progname, pid, num);
  (void) realpath(parentdir, testabs);
  (void) snprintf(pths->abs, PATH_MAX,
                  "%s/%s-%u-%d", testabs, progname, pid, num);
}

/* Get alternate group for testing */
/* This can be any of our groups other than the default */
static gid_t
get_testgrp(void)
{
  int err, i;
  uid_t ouruid = getuid();
  gid_t basegid, cur;
  struct passwd *pwd;
  int ngroups = MAX_GROUPS;
  int groups[MAX_GROUPS];

  /* We need our actual userid to use getgrouplist() */
  pwd = getpwuid(ouruid);
  if (!pwd) return 0;
  basegid = pwd->pw_gid;

  err = getgrouplist(pwd->pw_name, basegid, groups, &ngroups);
  /* Some errors "succeed" and don't store ngroups */
  if (err || ngroups == MAX_GROUPS) return 0;
  for (i = 0; i < ngroups; ++i) {
    cur = groups[i];
    if (cur != basegid) return cur;
  }
  return 0;
}

static int
check_cwd(int init)
{
  int cret;
  static char lastdir[PATH_MAX];
  char curdir[PATH_MAX];

  if (init) {
    if (!getcwd(lastdir, PATH_MAX)) {
      perror("    *** getcwd() failed");
      return -1;
    }
    return 0;
  }
  if (!getcwd(curdir, PATH_MAX)) {
    perror("    *** getcwd() failed");
    return -1;
  }
  cret = strncmp(curdir, lastdir, PATH_MAX);
  if (cret) {
    printf("    *** cwd changed: %s -> %s\n", lastdir, curdir);
  }

  (void) strncpy(lastdir, curdir, PATH_MAX);
  return cret != 0;
}

/* For debugging */
char *
get_cwd(void)
{
  static char dir[PATH_MAX];

  return getcwd(dir, PATH_MAX);
}

static int
get_stat(const char *path, struct stat *sb, int nofollow)
{
  int ret;

  if (nofollow) {
    ret = lstat(path, sb);
  } else {
    ret = stat(path, sb);
  }
  if (ret) {
    printf("  *** %s() for '%s' failed: %s\n",
           nofollow ? "lstat" : "stat", path, strerror(errno));
  }
  return ret;
}

static int
compare_stats(struct stat *sb1, struct stat *sb2)
{
  int ret = 0;

  if (sb1->st_dev != sb2->st_dev) {
    printf("  *** st_dev mismatches: %X != %X\n",
           (uint32_t) sb1->st_dev, (uint32_t) sb2->st_dev);
    ret = 1;
  }
  if (sb1->st_ino != sb2->st_ino) {
    printf("  *** st_ino mismatches: %llX != %llX\n",
           (unsigned long long) sb1->st_ino,
           (unsigned long long) sb2->st_ino);
    ret = 1;
  }
  return ret;
}

static int
check_nonex(const char *path, int nofollow)
{
  int ret;
  struct stat sb;

  if (nofollow) {
    ret = lstat(path, &sb);
  } else {
    ret = stat(path, &sb);
  }
  if (ret) {
    if (errno == ENOENT) return 0;
    printf("  *** unexpected error on nonex check for '%s': %s\n",
           path, strerror(errno));
    return 1;
  }
  printf("  *** '%s' exists and shouldn't\n", path);
  return 1;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, dirfd = -1, dirfd2 = -1, fd = -1;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  int i, count;
  gid_t testgid;
  struct stat sb1, sb2, sb3;
  char pathtmp[PATH_MAX];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", progname);

  for (i = 0; i < NUM_PATHS; ++i) {
    setpaths(&paths[i], progname, pid, i + 1, NULL);
  }
  setpaths(&paths[3], progname, pid, 3 + 1, &paths[2]);

  testgid = get_testgrp();

  ret = check_cwd(1);

  do {
    if (verbose) printf("  opening '%s' as test dir\n", TEST_TEMP);
    if ((dirfd = open(TEST_TEMP, O_RDONLY)) < 0) {
      printf("  *** unable to open '%s': %s\n", TEST_TEMP, strerror(errno));
      ret = 1;
      break;
    }

    if (verbose) printf("  openat() creating '%s'\n", paths[0].rel);
    if ((fd = openat(dirfd, paths[0].rel, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
      printf("  *** openat() for '%s' failed: %s\n",
             paths[0].rel, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[0].rel, 0);

    if (verbose) printf("  checking fstatat() with non-dir '%s/foo'\n",
                        paths[0].rel);
    if (fstatat(fd, "foo", &sb1, 0)) {
      if (errno != ENOTDIR) {
        printf("  *** fstatat() with non-dir '%s/foo' got wrong error: %s\n",
               paths[0].rel, strerror(errno));
        ret = 1;
        break;
      }
    } else {
      printf("  *** fstatat() with non-dir '%s/foo' succeeded\n",
             paths[0].rel);
      ret = 1;
    }
    ret |= check_cwd(0);

    if (close(fd)) {
      printf("  *** unable to close '%s': %s\n",
             paths[0].rel, strerror(errno));
      ret = 1;
    }
    fd = -1;

    if (verbose) printf("  symlinkat() '%s' -> '%s'\n",
                        paths[1].rel, paths[0].rel);
    if (symlinkat(paths[0].rel, dirfd, paths[1].rel)) {
      printf("  *** symlinkat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[0].rel, 1);

    if (verbose) printf("  readlinkat() checking symlink '%s' -> '%s'\n",
                        paths[1].rel, paths[0].rel);
    count = readlinkat(dirfd, paths[1].rel, pathtmp, sizeof(pathtmp));
    if (count < 0) {
      printf("  *** readlinkat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    if (count >= sizeof(pathtmp)) {
      printf("  *** readlinkat() length %d exceeded buffer size\n", count);
      ret = 1;
      break;
    }
    pathtmp[count] = '\0';
    if (strncmp(pathtmp, paths[0].rel, sizeof(pathtmp))) {
      printf ("  *** symlink value '%s' mismatched '%s'\n",
              pathtmp, paths[0].rel);
      ret = 1;
      break;
    }

    if (verbose) printf("  checking symlink target file matches\n");
    if (get_stat(paths[0].top, &sb1, 0)
        || get_stat(paths[1].top, &sb2, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

    if (verbose) printf("  checking fchmodat() for regular file\n");
    sb1.st_mode ^= S_IROTH;  /* Complement world read for test */
    if (fchmodat(dirfd, paths[0].rel, sb1.st_mode, 0)) {
      printf("  *** fchmodat() for '%s' failed: %s\n",
             paths[0].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    if (get_stat(paths[0].top, &sb3, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb3, &sb1)) {
      ret = 1;
      break;
    }

    if (verbose) printf("  checking fchmodat() for symlink\n");
    sb2.st_mode ^= S_IROTH;  /* Complement world read for test */
    if (fchmodat(dirfd, paths[1].rel, sb2.st_mode, AT_SYMLINK_NOFOLLOW)) {
      printf("  *** fchmodat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    if (get_stat(paths[1].top, &sb3, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb3, &sb2)) {
      ret = 1;
      break;
    }

    if (!testgid) {
      printf("  *** no alternate gid available for gid test\n");
    } else {

      if (verbose) printf("  checking fchownat() for regular file\n");
      sb1.st_gid = testgid;
      if (fchownat(dirfd, paths[0].rel, -1, testgid, 0)) {
        printf("  *** fchownat() for '%s' failed: %s\n",
               paths[0].top, strerror(errno));
        ret = 1;
        break;
      }
      ret |= check_cwd(0);
      if (get_stat(paths[0].top, &sb3, 0)) {
        ret = 1;
        break;
      }
      if (compare_stats(&sb3, &sb1)) {
        ret = 1;
        break;
      }

      if (verbose) printf("  checking fchownat() for symlink\n");
      sb2.st_mode ^= S_IROTH;  /* Complement world read for test */
      if (fchownat(dirfd, paths[1].rel, -1, testgid, AT_SYMLINK_NOFOLLOW)) {
        printf("  *** fchownat() for '%s' failed: %s\n",
               paths[1].top, strerror(errno));
        ret = 1;
        break;
      }
      ret |= check_cwd(0);
      if (get_stat(paths[1].top, &sb3, 0)) {
        ret = 1;
        break;
      }
      if (compare_stats(&sb3, &sb2)) {
        ret = 1;
        break;
      }
    }

    if (verbose) printf("  checking unlinkat() for symlink\n");
    if (unlinkat(dirfd, paths[1].rel, 0)) {
      printf("  *** unlinkat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].top, 1);

    if (verbose) printf("  checking mkfifoat()\n");
    if (mkfifoat(dirfd, paths[1].rel, S_IRWXU)) {
      printf("  *** mkfifoat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    if (verbose) printf("  checking unlinkat() for fifo\n");
    if (unlinkat(dirfd, paths[1].rel, 0)) {
      printf("  *** unlinkat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].top, 1);

    if (verbose) printf("  checking mknodat() for fifo\n");
    if (mknodat(dirfd, paths[1].rel, S_IFIFO | S_IRWXU, 0)) {
      printf("  *** mknodat() for fifo '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    if (verbose) printf("  checking unlinkat() for fifo node\n");
    if (unlinkat(dirfd, paths[1].rel, 0)) {
      printf("  *** unlinkat() for '%s' failed: %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].top, 1);

    if (verbose) printf("  checking mkdirat() '%s'\n", paths[2].rel);
    if (mkdirat(dirfd, paths[2].rel, S_IRWXU)) {
      printf("  *** mkdirat() for '%s' failed: %s\n",
             paths[2].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[2].rel, 0);
    paths[2].isdir = 1;
    if ((dirfd2 = open(paths[2].top, O_RDONLY)) < 0) {
      printf("  *** open() for '%s' failed: %s\n",
             paths[2].top, strerror(errno));
      ret = 1;
      break;
    }

    if (verbose) printf("  checking linkat() rel -> rel\n");
    if (linkat(dirfd, paths[0].rel, dirfd2, paths[3].rel, 0)) {
      printf("  *** linkat() for '%s' -> '%s' failed: %s\n",
             paths[0].top, paths[3].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[3].rel, 0);
    if (verbose) printf("  checking link target file matches\n");
    if (get_stat(paths[0].top, &sb1, 0)
        || get_stat(paths[3].top, &sb2, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

/*
 * Since linkat and renameat are almost entirely the same code, we mostly
 * test with renameat and assume that the one linkat test was sufficient.
 */

    if (verbose) printf("  checking renameat() rel ->abs\n");
    if (renameat(dirfd2, paths[3].rel, dirfd, paths[1].abs)) {
      printf("  *** renameat() for '%s' -> '%s' failed: %s\n",
             paths[3].top, paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].rel, 0);
    if (verbose) printf("  checking renamed file matches\n");
    if (get_stat(paths[1].abs, &sb1, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

    if (verbose) printf("  checking renameat() rel -> rel samedir\n");
    if (renameat(dirfd, paths[0].rel, dirfd, paths[1].rel)) {
      printf("  *** renameat() for '%s' -> '%s' failed: %s\n",
             paths[0].top, paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].rel, 0);
    if (verbose) printf("  checking renamed file matches\n");
    if (get_stat(paths[1].abs, &sb1, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

    if (verbose) printf("  checking renameat() abs -> rel\n");
    if (renameat(dirfd, paths[1].abs, dirfd, paths[0].rel)) {
      printf("  *** renameat() for '%s' -> '%s' failed: %s\n",
             paths[1].top, paths[0].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[0].rel, 0);
    if (verbose) printf("  checking renamed file matches\n");
    if (get_stat(paths[0].abs, &sb1, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

    if (verbose) printf("  checking renameat() abs ->abs\n");
    if (renameat(dirfd, paths[0].abs, dirfd, paths[1].abs)) {
      printf("  *** renameat() for '%s' -> '%s' failed: %s\n",
             paths[0].top, paths[1].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[1].rel, 0);
    if (verbose) printf("  checking renamed file matches\n");
    if (get_stat(paths[1].abs, &sb1, 0)) {
      ret = 1;
      break;
    }
    if (compare_stats(&sb1, &sb2)) {
      ret = 1;
      break;
    }

    if (close(dirfd2)) {
      printf("  *** unable to close '%s': %s\n",
             paths[1].top, strerror(errno));
      ret = 1;
    }
    dirfd2 = -1;

    if (verbose) printf("  checking unlinkat() for directory\n");
    if (unlinkat(dirfd, paths[2].rel, AT_REMOVEDIR)) {
      printf("  *** unlinkat() for '%s' failed: %s\n",
             paths[2].top, strerror(errno));
      ret = 1;
      break;
    }
    ret |= check_cwd(0);
    ret |= check_nonex(paths[2].top, 0);
  } while (0);

  if (verbose) printf("  cleaning up\n");
  if (fd >= 0) (void) close(fd);
  if (dirfd2 >= 0) (void) close(dirfd2);
  if (dirfd >= 0) (void) close(dirfd);
  for (i = NUM_PATHS -1; i >= 0; --i) {
    if (paths[i].isdir) {
      (void) rmdir(paths[i].top);
      (void) rmdir(paths[i].rel);  /* In case of screwup */
    } else {
      (void) unlink(paths[i].top);
      (void) unlink(paths[i].rel);  /* In case of screwup */
    }
  }

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
