/*
 * Copyright (c) 2019
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

#include <stdio.h>
#include <pthread.h>

int
main (void)
{
    printf("\nTesting PTHREAD_RWLOCK_INITIALIZER\n");
    pthread_rwlock_t  aLock = PTHREAD_RWLOCK_INITIALIZER;
    pthread_rwlock_rdlock(&aLock);
    pthread_rwlock_unlock(&aLock);
    pthread_rwlock_destroy(&aLock);
    printf("Success testing PTHREAD_RWLOCK_INITIALIZER\n\n");

    printf("Testing pthread_rwlock_init\n");
    pthread_rwlock_t  myLock;
    pthread_rwlock_init(&myLock, NULL);
    pthread_rwlock_rdlock(&myLock);
    pthread_rwlock_unlock(&myLock);
    pthread_rwlock_destroy(&myLock);
    printf("Success testing pthread_rwlock_init\n\n");

    return 0;
}
