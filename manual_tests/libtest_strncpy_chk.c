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
 * This provides test cases for the strncpy() security wrapper.
 * There may or may not be compile-time warnings in some cases, but
 * the final pass/fail should be as expected.
 *
 * This is included as a security-wrapper test of a function *not*
 * provided by legacy-support, for comparison purposes.  It's identical
 * to test_stpncpy_chk, except for s|stpncpy|strncpy|.
 *
 * Informational messages regarding the relevant compile flags are also
 * included.
 */

/* Allow deferring the stdio.h include */
int printf(const char *format, ...);

#define PRINT_VAR(x) printf("    " #x " = %lld\n", (long long) x)
#define PRINT_UNDEF(x) printf("    " #x " is undefined\n")
#define XPRINT_VAR(x) printf(#x " = %lld", (long long) x)
#define XPRINT_UNDEF(x) printf(#x " (undef)")

void
print_source(void)
{
  #ifdef _FORTIFY_SOURCE
  XPRINT_VAR(_FORTIFY_SOURCE);
  #else
  XPRINT_UNDEF(_FORTIFY_SOURCE);
  #endif
}

void
print_before(void)
{
  printf("  Before string.h include:\n");
  #ifdef _FORTIFY_SOURCE
  PRINT_VAR(_FORTIFY_SOURCE);
  #else
  PRINT_UNDEF(_FORTIFY_SOURCE);
  #endif
  #ifdef _USE_FORTIFY_LEVEL
  PRINT_VAR(_USE_FORTIFY_LEVEL);
  #else
  PRINT_UNDEF(_USE_FORTIFY_LEVEL);
  #endif
}

#include <string.h>

void
print_use(void)
{
  #ifdef _USE_FORTIFY_LEVEL
  XPRINT_VAR(_USE_FORTIFY_LEVEL);
  #else
  XPRINT_UNDEF(_USE_FORTIFY_LEVEL);
  #endif
}

void
print_after(void)
{
  printf("  After  string.h include:\n");
  #ifdef _FORTIFY_SOURCE
  PRINT_VAR(_FORTIFY_SOURCE);
  #else
  PRINT_UNDEF(_FORTIFY_SOURCE);
  #endif
  #ifdef _USE_FORTIFY_LEVEL
  PRINT_VAR(_USE_FORTIFY_LEVEL);
  #else
  PRINT_UNDEF(_USE_FORTIFY_LEVEL);
  #endif
}

#if defined(_USE_FORTIFY_LEVEL) && _USE_FORTIFY_LEVEL > 0
#define CHECKS_ARE_ENABLED 1
#else
#define CHECKS_ARE_ENABLED 0
#endif

#include <assert.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Try to disable warnings for things we're intentionally provoking */
#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wunknown-pragmas"
  #pragma clang diagnostic ignored "-Wbuiltin-memcpy-chk-size"
  #pragma clang diagnostic ignored "-Wunknown-warning-option"
  #pragma clang diagnostic ignored "-Wfortify-source"
#elif defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Wstringop-overflow"
  #pragma GCC diagnostic ignored "-Warray-bounds"
#endif

const char *test_str = "The Quick Brown Fox";

#define BUF_LEN 128  /* Generously longer than test_str */
static char dest[BUF_LEN];

/* Test strncpy() known good at compile time */
void
test_good_strncpy_static(void)
{
  (void) strncpy(dest, test_str, BUF_LEN);
}

/* Test strncpy() known bad at compile time */
void
test_bad_strncpy_static(void)
{
  (void) strncpy(dest, test_str, BUF_LEN+1);
}

/* Test strncpy() known good as auto buf */
void
test_good_strncpy_auto(void)
{
  char buf[BUF_LEN];

  (void) strncpy(buf, test_str, BUF_LEN);
}

/* Test strncpy() known bad as auto buf */
void
test_bad_strncpy_auto(void)
{
  char buf[BUF_LEN];

  (void) strncpy(buf, test_str, BUF_LEN+1);
}

/*
 * Some documentation suggests that the checks can work for cases where
 * the output buffer length isn't known at compile time.  Empirically,
 * this does not seem to be the case, though perhaps there are tools which
 * can make this work for testing.
 *
 * Meanwhile, we include a couple of tests for this, but disable them.
 */

/* Test strncpy() known good as runtime malloc() */
void
test_good_strncpy_runtime(void)
{
  char *buf;

  buf = malloc(BUF_LEN);
  if (!buf) {
    perror("malloc() failed");
    test_bad_strncpy_static();  /* Try to provoke failure */
    return;
  }
  (void) strncpy(buf, test_str, BUF_LEN);
  free(buf);
}

/* Test strncpy() known bad as runtime malloc() */
void
test_bad_strncpy_runtime(void)
{
  char *buf;

  buf = malloc(BUF_LEN);
  if (!buf) {
    perror("malloc() failed");
    return;
  }
  (void) strncpy(buf, test_str, BUF_LEN+1);
  free(buf);
}

typedef void test_func_t(void);

/* Run test func as a subprocess, to capture abort() */
int
run_test_func(test_func_t *func)
{
  pid_t child, done;
  int status;

  child = fork();
  if (child < 0) {
    perror("fork() failed");
    exit(100);
  }
  if (child == 0) {
    (*func)();
    exit(0);
  }
  done = wait(&status);
  if (done != child) {
    fprintf(stderr, "Unexpected wait() pid, %d != %d\n", done, child);
    exit(110);
  }
  return status;
}

int
main(int argc, char *argv[])
{
  int have_compile_time_checks = CHECKS_ARE_ENABLED;
  int have_runtime_checks = 0;  /* These don't currently work */
  int verbose = 0;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  printf("Running %s", basename(argv[0]));
  if (!verbose) {
    printf(", ");
    print_source();
    printf(" => ");
    print_use();
    printf("\n");
  } else {
    printf("\n");
    print_before();
    print_after();
  }

  /* Forking with unflushed buffers may produce duplicate output. */
  fflush(NULL);

  assert(run_test_func(&test_good_strncpy_static) == 0);
  assert(run_test_func(&test_good_strncpy_auto) == 0);
  if (have_compile_time_checks) {
    assert(run_test_func(&test_bad_strncpy_static) != 0);
    assert(run_test_func(&test_bad_strncpy_auto) != 0);
  } else {
    assert(run_test_func(&test_bad_strncpy_static) == 0);
    /* Apparently the "auto" check happens even when nominally disabled. */
    /* assert(run_test_func(&test_bad_strncpy_auto) == 0); */
  }
  if (have_runtime_checks) {
    assert(run_test_func(&test_good_strncpy_runtime) == 0);
    assert(run_test_func(&test_bad_strncpy_runtime) != 0);
  }

  if (verbose) printf("Tests pass\n");
  return 0;
}
