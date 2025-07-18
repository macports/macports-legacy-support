/*
 * Copyright (C) 2023 raf <raf@raf.org>
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
Test directory traversal with MacPorts legacy-support.

This test creates a temporary directory <temp_dir>, <temp_dir>/subdir,
and <temp_dir>/subdir/file.
It then chdirs into <temp_dir> and then traverses ".".
It then chdirs to ".." afterwards.
It then deletes <temp_dir> and its contents.

The (verbose) output should look something like:

  cd <temp_dir>
  fstatat(parent_fd=-2, .) ok
  openat(parent_fd=-2, .) = dir_fd=3 ok
  fdopendir(dir_fd=3) ok
  entry subdir
  fstatat(parent_fd=3, subdir) ok
  openat(parent_fd=3, subdir) = dir_fd=4 ok
  fdopendir(dir_fd=4) ok
  entry file
  fstatat(parent_fd=4, file) ok
  cwd (before cd ..) <test_data_dir>/<temp_dir>
  cwd (after cd ..)  <test_data_dir>
  cd <build_dir>

This differs from test/test_traverse.c which traverses a
named directory rather than ".". Originally, these
exhibited different errors.

*/

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

static char topdir[MAXPATHLEN];
static char subdir[MAXPATHLEN];
static char testfile[MAXPATHLEN];

static int
traverse(int parent_fd, const char *name, int verbose)
{
  int dir_fd;
  DIR *dir;

  /* Test: fstatat(AT_FDCWD, <temp_dir>) */

  struct stat statbuf[1];

  if (fstatat(parent_fd, name, statbuf, AT_SYMLINK_NOFOLLOW) == -1)
  {
    fprintf(stderr, "fstatat(parent_fd=%d, %s) failed: %s\n", parent_fd, name, strerror(errno));
    return EXIT_FAILURE;
  }

  if (verbose) printf("fstatat(parent_fd=%d, %s) ok\n", parent_fd, name);

  /* If it's a directory, process its entries */

  if ((statbuf->st_mode & S_IFMT) == S_IFDIR)
  {
    /* Open it with openat() */

    if ((dir_fd = openat(parent_fd, name, O_RDONLY)) == -1)
    {
      fprintf(stderr, "openat(parent_fd=%d, %s) failed: %s\n", parent_fd, name, strerror(errno));
      return EXIT_FAILURE;
    }

    if (verbose) {
      printf("openat(parent_fd=%d, %s) = dir_fd=%d ok\n",
             parent_fd, name, dir_fd);
    }

    /* Open it for traversing with fdopendir() */

    if (!(dir = fdopendir(dir_fd)))
    {
      fprintf(stderr, "fdopendir(dir_fd=%d, dir) failed\n", dir_fd);
      close(dir_fd);
      return EXIT_FAILURE;
    }

    if (verbose) printf("fdopendir(dir_fd=%d) ok\n", dir_fd);

    /* Apply recursively to this directory's entries */

    struct dirent *entry;

    while ((entry = readdir(dir)))
    {
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        continue;

      if (verbose) printf("entry %s\n", entry->d_name);

      if (traverse(dir_fd, entry->d_name, verbose) == EXIT_FAILURE)
        return EXIT_FAILURE;
    }

    closedir(dir);
  }

  return EXIT_SUCCESS;
}

/* Cleanup */

static void
cleanup(int quiet)
{

  if (unlink(testfile) == -1 && !quiet) {
    fprintf(stderr, "unlink(%s) failed: %s\n", testfile, strerror(errno));
  }

  if (rmdir(subdir) == -1 && !quiet) {
    fprintf(stderr, "unlink(%s) failed: %s\n", subdir, strerror(errno));
  }

  if (rmdir(topdir) == -1 && !quiet) {
    fprintf(stderr, "unlink(%s) failed: %s\n", topdir, strerror(errno));
  }
}

/* Setup */

static int
setup(void)
{
  int fd;

  cleanup(1);

  /* Prepare: Create the top directory */

  if (mkdir(topdir, (mode_t) 0755) == -1)
  {
    fprintf(stderr, "mkdir(%s) failed: %s\n", topdir, strerror(errno));
    return EXIT_FAILURE;
  }

  /* And a directory within it */

  if (mkdir(subdir, (mode_t) 0755) == -1)
  {
    fprintf(stderr, "mkdir(%s) failed: %s\n", subdir, strerror(errno));
    cleanup(1);
    return EXIT_FAILURE;
  }

  /* And a file within that */

  if ((fd = creat(testfile, (mode_t) 0644)) == -1)
  {
    fprintf(stderr, "creat(%s) failed: %s\n", testfile, strerror(errno));
    cleanup(1);
    return EXIT_FAILURE;
  }

  (void) close(fd);

  return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
  int rc, verbose = 0;
  pid_t pid = getpid();
  char *progname = basename(argv[0]);
  char cwdbuf0[MAXPATHLEN];
  char cwdbuf1[MAXPATHLEN];
  char cwdbuf2[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(topdir, sizeof(topdir), TEST_TEMP "/%s-%u", progname, pid);
  (void) snprintf(subdir, sizeof(subdir), "%s/subdir", topdir);
  (void) snprintf(testfile, sizeof(testfile), "%s/file", subdir);

  (void) getcwd(cwdbuf0, sizeof(cwdbuf0));

  /* Prepare: Create the directories and file */

  rc = setup();

  if (!rc) {
    /* Test directory traversal */

    if (verbose) printf("cd %s\n", topdir);
    if (chdir(topdir) == -1)
      fprintf(stderr, "chdir(%s) failed: %s\n", topdir, strerror(errno));

    rc = traverse(AT_FDCWD, ".", verbose);

    (void) getcwd(cwdbuf1, sizeof(cwdbuf1));
    if (verbose) printf("cwd (before cd ..) %s\n", cwdbuf1);

    if (chdir("..") == -1)
      fprintf(stderr, "chdir(\"..\") failed: %s\n", strerror(errno));

    (void) getcwd(cwdbuf2, sizeof(cwdbuf2));
    if (verbose) printf("cwd (after cd ..) %s\n", cwdbuf2);

    if (!strcmp(cwdbuf1, cwdbuf2)) {
      fprintf(stderr, "Post-traversal chdir(..) silently failed to change directory!\n");
      fprintf(stderr, "Replacing __mpls_best_fchdir() in fdopendir() with fchdir() fixes this badly.\n");
      fprintf(stderr, "Using _ATCALL for opendir fixes this properly.\n");
      rc = EXIT_FAILURE;
    }

    if (verbose) printf("cd %s\n", cwdbuf0);
    if (chdir(cwdbuf0) == -1)
      fprintf(stderr, "chdir(%s) failed: %s\n", cwdbuf0, strerror(errno));

    cleanup(0);
  }

  printf("%s %s.\n", progname, rc ? "failed" : "passed");
  return (rc) ? 1 : 0;
}
