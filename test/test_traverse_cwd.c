
/*
 * Copyright (C) 2023 raf <raf@raf.org>
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
Test directory traversal with macports legacysupport.

This test creates ".test.dir", ".test.dir/subdir" and ".test.dir/subdir/file".
It then chdirs into ".test.dir" and then traverses ".".
It then chdirs to ".." afterwards.
It then deletes ".test.dir" and its contents.

The output should look something like:

  fstatat(parent_fd=-2, .) ok
  openat(parent_fd=-2, .) = dir_fd=3 ok
  fdopendir(dir_fd=3) ok
  entry subdir
  fstatat(parent_fd=3, subdir) ok
  openat(parent_fd=3, subdir) = dir_fd=4 ok
  fdopendir(dir_fd=4) ok
  entry file
  fstatat(parent_fd=4, file) ok
  cwd (before cd ..) /Users/.../macports-legacy-support/.test.dir
  cwd (after cd ..)  /Users/.../macports-legacy-support

This differs from test/test_traverse.c which traverses a
named directory rather than ".". Originally, these
exhibited different errors.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

int traverse(int parent_fd, const char *name)
{
    /* Test: fstatat(AT_FDCWD, .test.dir) */

    struct stat statbuf[1];

    if (fstatat(parent_fd, name, statbuf, AT_SYMLINK_NOFOLLOW) == -1)
    {
        fprintf(stderr, "fstatat(parent_fd=%d, %s) failed: %s\n", parent_fd, name, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("fstatat(parent_fd=%d, %s) ok\n", parent_fd, name);

    /* If it's a directory, process its entries */

    if ((statbuf->st_mode & S_IFMT) == S_IFDIR)
    {
        /* Open it with openat() */

        int dir_fd;

        if ((dir_fd = openat(parent_fd, name, O_RDONLY)) == -1)
        {
            fprintf(stderr, "openat(parent_fd=%d, %s) failed: %s\n", parent_fd, name, strerror(errno));
            return EXIT_FAILURE;
        }

        printf("openat(parent_fd=%d, %s) = dir_fd=%d ok\n", parent_fd, name, dir_fd);

        /* Open it for traversing with fdopendir() */

        DIR *dir;

        if (!(dir = fdopendir(dir_fd)))
        {
            fprintf(stderr, "fdopendir(dir_fd=%d, dir) failed\n", dir_fd);
            close(dir_fd);
            return EXIT_FAILURE;
        }

        printf("fdopendir(dir_fd=%d) ok\n", dir_fd);

        /* Apply recursively to this directory's entries */

        struct dirent *entry;

        while ((entry = readdir(dir)))
        {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            printf("entry %s\n", entry->d_name);

            if (traverse(dir_fd, entry->d_name) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }

        closedir(dir);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    /* Prepare: Create a directory */

    system("rm -rf .test.dir");

    if (mkdir(".test.dir", (mode_t)0755) == -1)
    {
        perror("mkdir(.test.dir) failed\n");
        exit(EXIT_FAILURE);
    }

    /* And a directory within it */

    if (mkdir(".test.dir/subdir", (mode_t)0755) == -1)
    {
        perror("mkdir(.test.dir/subdir) failed\n");
        (void)rmdir(".test.dir");
        exit(EXIT_FAILURE);
    }

    /* And a file within that */

    int fd;

    if ((fd = creat(".test.dir/subdir/file", (mode_t)0644)) == -1)
    {
        perror("creat(.test.dir/subdir/file) failed\n");
        (void)rmdir(".test.dir/subdir");
        (void)rmdir(".test.dir");
        exit(EXIT_FAILURE);
    }

    close(fd);

    /* Test directory traversal */

    fprintf(stderr, "cd .test.dir\n");
    if (chdir(".test.dir") == -1)
        perror("chdir(.test.dir) failed");

    int rc = traverse(AT_FDCWD, ".");

    char cwdbuf1[BUFSIZ];
    char cwdbuf2[BUFSIZ];

    printf("cwd (before cd ..) %s\n", getcwd(cwdbuf1, BUFSIZ));

    if (chdir("..") == -1)
        perror("chdir .. failed");

    printf("cwd (after cd ..)  %s\n", getcwd(cwdbuf2, BUFSIZ));

    if (!strcmp(cwdbuf1, cwdbuf2))
    {
        fprintf(stderr, "Post-traversal chdir(..) silently failed to change directory!\n");
        fprintf(stderr, "Replacing best_fchdir() in fdopendir() with fchdir() fixes this badly.\n");
        fprintf(stderr, "Using _ATCALL for opendir fixes this properly.\n");
        rc = EXIT_FAILURE;
    }

    /* Cleanup */

    if (unlink(".test.dir/subdir/file") == -1)
        perror("unlink .test.dir/subdir/file failed");

    if (rmdir(".test.dir/subdir") == -1)
        perror("rmdir .test.dir/subdir failed");

    if (rmdir(".test.dir") == -1)
        perror("rmdir .test.dir failed");

    /* If the above cleanup didn't work (because chdir .. silently failed to work) */

    if (!strcmp(cwdbuf1, cwdbuf2))
    {
        fprintf(stderr, "Cleaning up\n");
        (void)unlink("../.test.dir/subdir/file");
        (void)rmdir("../.test.dir/subdir");
        (void)rmdir("../.test.dir");
    }

    return rc;
}

