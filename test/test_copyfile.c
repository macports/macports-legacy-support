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
 * This provides a limited test of copyfile(), mainly to test the operation
 * of copyfile_state_get() for COPYFILE_STATE_COPIED, which is added by
 * legacy-support in some cases.
 */

#include <copyfile.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

/* Secret debug flag for copyfile() */
#define COPYFILE_DEBUG (1U<<31)

/* Dummy copyfile context to test set/get */
typedef struct dummy_ctx_s {
  int dummy;
} dummy_ctx_t;

int
main(int argc, char *argv[])
{
  int verbose = 0, debug = 0;
  char *debugenv;
  copyfile_state_t state;
  dummy_ctx_t ctx = {0}, *ctxp;
  pid_t pid = getpid();
  char *name = basename(argv[0]);
  off_t copied;
  struct stat ourstat;
  char dest[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;
  if (argc > 1 && !strcmp(argv[1], "-d")) {
    verbose = 1;
    debug = COPYFILE_DEBUG;
  }

  (void) snprintf(dest, sizeof(dest), "%s/%s-%u", TEST_TEMP, name, pid);

  if (verbose) {
    printf("%s starting.\n", name);
    printf("  %s -> %s\n", argv[0], dest);
  }
  if (debug) {
    if ((debugenv = getenv("COPYFILE_DEBUG"))) {
      printf("    Debugging enabled, level (COPYFILE_DEBUG) = %s\n", debugenv);
    } else {
      printf("    Debugging enabled, COPYFILE_DEBUG env var not set\n");
    }
  }

  if (stat(argv[0], &ourstat)) {
    perror("unable to stat() self");
    return 1;
  }
  if (verbose) printf("  size is %ld bytes\n", (long) ourstat.st_size);

  state = copyfile_state_alloc();
  if (!state) {
    perror("copyfile_state_alloc() failed");
    return 1;
  }

  /* Check that we can set the context pointer */
  if (copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, &ctx)) {
    perror("unable to set COPYFILE_STATE_STATUS_CTX");
    return 1;
  }

  if (copyfile(argv[0], dest, state, COPYFILE_ALL | debug)) {
    perror("copyfile() failed");
    return 1;
  }

  if (copyfile_state_get(state, COPYFILE_STATE_COPIED, &copied)) {
    perror("unable to get COPYFILE_STATE_COPIED");
    return 1;
  }
  if (copied != ourstat.st_size) {
    fprintf(stderr, "  COPYFILE_STATE_COPIED = %ld bytes"
                    " mismatches st_size = %ld bytes\n",
            (long) copied, (long) ourstat.st_size);
    return 1;
  }

  /* Check that we can get the context pointer */
  if (copyfile_state_get(state, COPYFILE_STATE_STATUS_CTX, &ctxp)) {
    perror("unable to get COPYFILE_STATE_STATUS_CTX");
    return 1;
  }
  /* Check that it matches what we set */
  if (ctxp != &ctx) {
    fprintf(stderr, "  COPYFILE_STATE_STATUS_CTX set to 0x%lX"
                    " reads back as 0x%lX\n",
            (unsigned long) &ctx, (unsigned long) ctxp);
    return 1;
  }

  if (copyfile_state_free(state)) {
    perror("copyfile_state_free() failed");
    return 1;
  }

  printf("%s succeeded.\n", name);
  return 0;
}