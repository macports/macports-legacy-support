
/*
 * Copyright (c) 2019
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

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

/* Test expected failure case */

int check_failure(int fd, const char *name, const char *exp_sym, int exp_val)
{
    DIR *dir;

    if ((dir = fdopendir(fd))) {
        fprintf(stderr, "error: fdopendir(%s) should have failed\n", name);
        (void)closedir(dir);
        return 1;
    } else if (errno != exp_val) {
    fprintf(stderr, "error: fdopendir(%s) failure should have returned"
                    " %d (%s), actually returned %d (%s)\n",
                    name, exp_val, exp_sym, errno, strerror(errno));
        return 1;
    }
    return 0;
}

int main() {
    struct stat st;
    struct dirent *entry;
    int dfd = -1;
    DIR *dir;
    char *first_entry = NULL;
    int err;
    int pipefds[2];

    /* Test fdopendir with a valid directory fd, then use readdir */

    dfd = open(".", O_RDONLY);
    dir = fdopendir(dfd);
    if (!dir) {
        perror("error: fdopendir failed");
        fprintf(stderr, "dfd=%i\n", dfd);
        return 1;
    }

    while ((entry = readdir(dir))) {

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (!first_entry)
            first_entry = strdup(entry->d_name); /* Remember for rewinddir test later */
        if (!first_entry) {
            perror("error: strdup failed");
            (void)closedir(dir);
            return 1;
        }

        if (fstatat(dfd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0) {
            perror("error: fstatat after fdopendir failed");
            fprintf(stderr, "dfd=%i d_name=%s\n", dfd, entry->d_name);
            free(first_entry);
            (void)closedir(dir);
            return 1;
        }

#ifdef FEEDBACK
        printf("%s\n", entry->d_name);
#endif
    }

    /*
     * Test wrapper functions/macros (all but readdir_r which is deprecated)
     *
     * Although the wrapper functions that these tests were originally
     * intended for no longer exist, these same tests are also used to
     * check the transparent wrappers added for compatibility with old
     * client builds.
     */

    int dfd2 = dirfd(dir);
    if (dfd2 != dfd) {
        fprintf(stderr, "error: dirfd failed: %d != %d\n", dfd2, dfd);
        free(first_entry);
        (void)closedir(dir);
        return 1;
    }

    rewinddir(dir);

    long loc = telldir(dir);
    if (loc < 0) {
        perror("error: telldir failed");
        free(first_entry);
        (void)closedir(dir);
        return 1;
    }

    if (first_entry) {
        int found_first = 0;

        while ((entry = readdir(dir))) {

            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            if (strcmp(entry->d_name, first_entry)) {
                fprintf(stderr, "error: rewinddir failed: readdir %s != %s\n", entry->d_name, first_entry);
                free(first_entry);
                (void)closedir(dir);
                return 1;
            }

            found_first = 1;
            break;
        }

        if (!found_first) {
            fprintf(stderr, "rewinddir failed to rewind\n");
            free(first_entry);
            (void)closedir(dir);
            return 1;
        }
    }

    seekdir(dir, loc);

    if (first_entry) {
        int found_first = 0;

        while ((entry = readdir(dir))) {

            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            if (strcmp(entry->d_name, first_entry)) {
                fprintf(stderr, "error: rewinddir/telldir/readdir/seekdir failed: readdir %s != %s\n", entry->d_name, first_entry);
                free(first_entry);
                (void)closedir(dir);
                return 1;
            }

            found_first = 1;
            break;
        }

        if (!found_first) {
            fprintf(stderr, "seekdir failed to seek\n");
            free(first_entry);
            (void)closedir(dir);
            return 1;
        }
    }

    free(first_entry);
    first_entry = NULL;
    (void)closedir(dir);

    dir = opendir(".");
    if (!dir) {
        perror("error: opendir(.) failed");
        return 1;
    }

    while ((entry = readdir(dir))) {

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (fstatat(dfd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0) {
            perror("error: fstatat after opendir failed");
            fprintf(stderr, "dfd=%i d_name=%s\n", dfd, entry->d_name);
            (void)closedir(dir);
            return 1;
        }
    }

    (void)closedir(dir);

    /* Try to use fdopendir with stdin - Should fail with ENOTDIR */

    err = check_failure(STDIN_FILENO, "stdin", "ENOTDIR", ENOTDIR);
    if (err) return 1;

    /* Try to use fdopendir with -1 - Should fail with EBADF */

    err = check_failure(-1, "-1", "EBADF", EBADF);
    if (err) return 1;

    /* Try to use fdopendir with AT_FDCWD - Should fail with EBADF */

    err = check_failure(AT_FDCWD, "AT_FDCWD", "EBADF", EBADF);
    if (err) return 1;

    /*
     * Try to use fdopendir with pipe - Should fail with ENOTDIR
     *
     * This reproduces the stdin test failure seen when running the test via
     * a pipe, rather than a tty or pty.
     */

    if (pipe(pipefds) < 0) {
        perror("Unable to create pipe for pipe test");
        return 1;
    }
    (void) close(pipefds[1]);  /* Close the writing side immediately */

    err = check_failure(pipefds[0], "pipe", "ENOTDIR", ENOTDIR);
    (void) close(pipefds[0]);
    if (err) return 1;

    /* Also try with closed pipe - Should fail with EBADF */

    err = check_failure(pipefds[0], "closed", "EBADF", EBADF);
    if (err) return 1;

    return 0;
}

