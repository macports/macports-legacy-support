
/*
 * Copyright (c) 2019
 * Copyright (c) 2023 raf <raf@raf.org>
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
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define FEEDBACK 1

int main() {
    struct stat st;
    struct dirent *entry;
    int dfd = -1;
    DIR *dir;

    dfd = open(".", O_RDONLY);
    dir = fdopendir(dfd);
    if (!dir) {
        printf("error: fdopendir failed\n");
        printf("dfd: %i\n", dfd);
        return 1;
    }

    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        if (fstatat(dfd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0) {
            printf("error: fstatat failed on %s\n", entry->d_name);
            printf("dfd: %i\n", dfd);
            close(dfd);
            return 1;
        }
#if FEEDBACK
        printf("%s\n", entry->d_name);
#endif
    }

    close(dfd);

    /* Try to use fdopendir with stdin - Should fail with ENOTDIR */

    if ((dir = fdopendir(STDIN_FILENO))) {
        fprintf(stderr, "fdopendir(stdin) should have failed\n");
        (void)closedir(dir);
        return 1;
    } else if (errno != ENOTDIR) {
        perror("fdopendir(stdin) should have failed with ENOTDIR");
        return 1;
    }

    /* Try to use fdopendir with -1 - Should fail with EBADF */

    if ((dir = fdopendir(-1))) {
        fprintf(stderr, "fdopendir(-1) should have failed\n");
        (void)closedir(dir);
        return 1;
    } else if (errno != EBADF) {
        perror("fdopendir(-1) should have failed with EBADF");
        return 1;
    }

    return 0;
}

