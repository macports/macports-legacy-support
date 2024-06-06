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
 * This provides test cases for the stpncpy() security wrapper.
 * There may or may not be compile-time warnings in some cases, but
 * the final pass/fail should be as expected.
 *
 * Informational messages regarding the relevant compile flags are also
 * included.
 */

/* Allow deferring the stdio.h include */
int printf(const char *format, ...);

#define PRINT_VAR(x) printf("    %s = %lld\n", #x, (long long) x)
#define PRINT_UNDEF(x) printf("    " #x " is undefined\n")

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
#include <unistd.h>
#include <sys/wait.h>

const char *test_str = "The Quick Brown Fox";

#define BUF_LEN 128  /* Generously longer than test_str */
static char dest[BUF_LEN];

/* Test stpncpy() known good at compile time */
void
test_good_stpncpy_static(void)
{
  (void) stpncpy(dest, test_str, BUF_LEN);
}

/* Test stpncpy() known bad at compile time */
void
test_bad_stpncpy_static(void)
{
  (void) stpncpy(dest, test_str, BUF_LEN+1);
}

/* Test stpncpy() known good as auto buf */
void
test_good_stpncpy_auto(void)
{
  char buf[BUF_LEN];

  (void) stpncpy(buf, test_str, BUF_LEN);
}

/* Test stpncpy() known bad as auto buf */
void
test_bad_stpncpy_auto(void)
{
  char buf[BUF_LEN];

  (void) stpncpy(buf, test_str, BUF_LEN+1);
}

/*
 * Some documentation suggests that the checks can work for cases where
 * the output buffer length isn't known at compile time.  Empirically,
 * this does not seem to be the case, though perhaps there are tools which
 * can make this work for testing.
 *
 * Meanwhile, we include a couple of tests for this, but disable them.
 */

/* Test stpncpy() known good as runtime malloc() */
void
test_good_stpncpy_runtime(void)
{
  char *buf;

  buf = malloc(BUF_LEN);
  if (!buf) {
    perror("malloc() failed");
    test_bad_stpncpy_static();  /* Try to provoke failure */
    return;
  }
  (void) stpncpy(buf, test_str, BUF_LEN);
  free(buf);
}

/* Test stpncpy() known bad as runtime malloc() */
void
test_bad_stpncpy_runtime(void)
{
  char *buf;

  buf = malloc(BUF_LEN);
  if (!buf) {
    perror("malloc() failed");
    return;
  }
  (void) stpncpy(buf, test_str, BUF_LEN+1);
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

  (void) argc;

  printf("Running %s\n", basename(argv[0]));
  print_before();
  print_after();
  /* Forking with unflushed buffers may produce duplicate output. */
  fflush(NULL);

  assert(run_test_func(&test_good_stpncpy_static) == 0);
  assert(run_test_func(&test_good_stpncpy_auto) == 0);
  if (have_compile_time_checks) {
    assert(run_test_func(&test_bad_stpncpy_static) != 0);
    assert(run_test_func(&test_bad_stpncpy_auto) != 0);
  } else {
    assert(run_test_func(&test_bad_stpncpy_static) == 0);
    /* Apparently the "auto" check happens even when nominally disabled. */
    /* assert(run_test_func(&test_bad_stpncpy_auto) == 0); */
  }
  if (have_runtime_checks) {
    assert(run_test_func(&test_good_stpncpy_runtime) == 0);
    assert(run_test_func(&test_bad_stpncpy_runtime) != 0);
  }

  printf("Tests pass\n");
  return 0;
}
