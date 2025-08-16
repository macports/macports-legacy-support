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
 * This provides rudimentary tests for stpncpy().  Given that
 * the implementation is taken almost verbatim from Apple's code for 10.7+,
 * exhaustive testing of corner cases shouldn't be necessary.  This just
 * tests a few cases, primarily to verify that the function can be used at all
 * (which is not true prior to 10.7 without this package), though the framework
 * should be valid for all possible test cases.
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

const char *test_str = "The Quick Brown Fox";

#define BUF_LEN 128  /* Generously longer than test_str */

static int save_errno;
static char *save_ret;
static char dest[BUF_LEN], save_dest[BUF_LEN];

/* Reference implementation of stpncpy() */
static char *
ref_stpncpy(char *dst, const char *src, size_t n)
{
  char *ret;

  if (n <= 0) return dst;

  while (n) {
    if ((*dst++ = *src++) == 0) break;
    --n;
  }
  ret = dst - 1;
  while (n--) *dst++ = 0;
  return *ret ? ret + 1 : ret;
}

/* Setup prior to test case */
static void
setup(void)
{
  errno = 0;
  memset(dest, -1, BUF_LEN);
}

/* Save all results from first version */
static void
save(char *ret)
{
  save_errno = errno;
  save_ret = ret;
  memcpy(save_dest, dest, BUF_LEN);
}

/* Compare results from both versions */
static void
check(char *ret)
{
  assert(errno == save_errno);
  assert(ret == save_ret);
  assert(memcmp(save_dest, dest, BUF_LEN) == 0);
}

/* Run one test case */
static void
test_stpncpy(char *dst, const char *src, size_t n)
{
  setup();
  save(ref_stpncpy(dst, src, n));
  check(stpncpy(dst, src, n));
}

int
main(int argc, char *argv[])
{
  int test_len = strlen(test_str);
  int n;

  (void) argc; (void) argv;

  for (n = test_len - 2; n <= test_len + 2; ++n) {
    test_stpncpy(&dest[1], test_str, n);
  }

  return 0;
}
