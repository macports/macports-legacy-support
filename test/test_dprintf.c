/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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
 * This provides functional tests for dprintf, including *not* closing the
 * provided fd.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 256

#define TEST_ARGS "%s output is %d\n", name, 42

typedef int test_func_t(int fd, const char * __restrict format, ...);

static void
test_xdprintf(const char *name, test_func_t func, int pipes[], int verbose)
{
  int ret, explen, done = 0;
  ssize_t actlen;
  char expbuf[BUF_SIZE], actbuf[BUF_SIZE];

  if (verbose) {
    printf("Testing %s...\n", name);
    fflush(stdout);
  }

  /* Create expected result */
  explen = snprintf(expbuf, sizeof(expbuf), TEST_ARGS);
  assert(explen > 0 && "snprintf failed creating expected");

  /* Test func writing to wrong (read) end of pipe */
  ret = func(pipes[0], TEST_ARGS);
  assert(ret < 0 && "Bad return value with invalid fd");
  assert(errno == EBADF && "Bad errno with invalid fd");

  /* Do test twice to detect unexpected close() */
  do {
    /* Test func writing to correct (write) end of pipe */
    errno = 0;
    ret = func(pipes[1], TEST_ARGS);
    assert(ret == explen && "Bad return value with valid fd");
    assert(errno == 0 && "Bad errno with valid fd");

    /* Read and check the pipe data */
    actlen = read(pipes[0], actbuf, sizeof(actbuf));
    assert(actlen == explen && "Incorrect length read from pipe");
    assert(!strncmp(actbuf, expbuf, explen) && "Piped result doesn't match");
  } while (!done++);
}

int
main(int argc, const char *argv[]) {
  int verbose = 0, pipes[2];

  if (argc >= 2 && !strcmp(argv[1], "-v")) verbose = 1;

  if (pipe(pipes)) {
    perror("Unable to create pipe");
    return 1;
  }

  test_xdprintf("dprintf", dprintf, pipes, verbose);

  if (close(pipes[1])) {
    perror("Unable to close write pipe");
    return 1;
  }
  if (close(pipes[0])) {
    perror("Unable to close read pipe");
    return 1;
  }

  printf("dprintf test succeeded\n");
  return 0;
}
