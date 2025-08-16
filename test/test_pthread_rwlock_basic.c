/*
 * Copyright (c) 2019
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

#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    int ret = 0, verbose = 0;

    if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

    /*
     * Note that this doesn't actually test anything, besides surviving
     * the calls.
     */

    if (verbose) printf("Testing PTHREAD_RWLOCK_INITIALIZER\n");
    pthread_rwlock_t  aLock = PTHREAD_RWLOCK_INITIALIZER;
    pthread_rwlock_rdlock(&aLock);
    pthread_rwlock_unlock(&aLock);
    pthread_rwlock_destroy(&aLock);
    if (verbose) printf("  Success testing PTHREAD_RWLOCK_INITIALIZER\n");

    if (verbose) printf("Testing pthread_rwlock_init\n");
    pthread_rwlock_t  myLock;
    pthread_rwlock_init(&myLock, NULL);
    pthread_rwlock_rdlock(&myLock);
    pthread_rwlock_unlock(&myLock);
    pthread_rwlock_destroy(&myLock);
    if (verbose) printf("  Success testing pthread_rwlock_init\n");

    printf("%s %s.\n", basename(argv[0]), ret ? "failed" : "passed");
    return ret;
}
