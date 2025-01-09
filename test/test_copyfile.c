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
#include <sys/sysctl.h>
#include <sys/types.h>

/* sysctl to check whether we're running natively (not Rosetta) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* Set up condition for testing the compatibility wrappers. */
#if !defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050
#define TEST_TIGER 1
#else
#define TEST_TIGER 0
#endif

#if TEST_TIGER
/* Avoid problematic malloc.h by direct declaration of malloc_size(). */
extern size_t malloc_size(const void *ptr);
/* Prototypes in case we build with later SDK */
int copyfile_free(copyfile_state_t);
copyfile_state_t copyfile_init(void);
#endif

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

/* Secret debug flag for copyfile() */
#define COPYFILE_DEBUG (1U<<31)

/* Dummy copyfile context to test set/get */
typedef struct dummy_ctx_s {
  int dummy;
} dummy_ctx_t;

/*
 * 10.4 Rosetta is unable to handle COPYFILE_ACL, so we need to check.
 */
#if TEST_TIGER && defined(__ppc__)
static int
arch_ok(void)
{
  int val = 0;
  size_t vsiz = sizeof(val);

  if (sysctlbyname(SYSCTL_NATIVE, &val, &vsiz, NULL, 0) < 0) return -1;
  return val;
}
#else /* not possibly 10.4 Rosetta */
static int arch_ok(void) { return 1;}
#endif

int
main(int argc, char *argv[])
{
  int verbose = 0;
  copyfile_flags_t test_flags = COPYFILE_ALL;
  char *debugenv;
  copyfile_state_t state, state_test;
  dummy_ctx_t ctx = {0}, *ctxp;
  pid_t pid = getpid();
  char *name = basename(argv[0]);
  off_t copied;
  struct stat ourstat;
  char dest[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;
  if (argc > 1 && !strcmp(argv[1], "-d")) {
    verbose = 1;
    test_flags |= COPYFILE_DEBUG;
  }

  (void) snprintf(dest, sizeof(dest), "%s/%s-%u", TEST_TEMP, name, pid);

  if (verbose) {
    printf("%s starting.\n", name);
    printf("  %s -> %s\n", argv[0], dest);
  }
  if (test_flags & COPYFILE_DEBUG) {
    if ((debugenv = getenv("COPYFILE_DEBUG"))) {
      printf("    Debugging enabled, level (COPYFILE_DEBUG) = %s\n", debugenv);
    } else {
      printf("    Debugging enabled, COPYFILE_DEBUG env var not set\n");
    }
  }

  if (!arch_ok()) {
    if (verbose) printf("    Avoiding COPYFILE_ACL due to Rosetta bug\n");
    test_flags &= ~COPYFILE_ACL;
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

/* Verify that the old allocator is really wrapping the new one. */
#if TEST_TIGER
  state_test = copyfile_init();
  if (!state_test) {
    perror("copyfile_init() failed");
    return 1;
  }
  if (malloc_size(state_test) != malloc_size(state)) {
    fprintf(stderr, "copyfile_init() size %d mismatches"
                    " copyfile_state_alloc() size %d\n",
            (int) malloc_size(state_test), (int) malloc_size(state));
    return 1;
  }
  copyfile_free(state_test);
#else
  (void) state_test;
#endif

  /* Check that we can set the context pointer */
  if (copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, &ctx)) {
    perror("unable to set COPYFILE_STATE_STATUS_CTX");
    return 1;
  }

  if (copyfile(argv[0], dest, state, test_flags)) {
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
