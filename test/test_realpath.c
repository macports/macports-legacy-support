/*
 * Copyright (c) 2019 Christian Cornelssen
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
 * Deliberately declaring some potentially redefined names
 * before including the associated header file, to test robustness.
 */

/* Wrapper should work not only with calls, but with references as well */
/* __restrict not needed here (and remember: nothing included yet) */
typedef char * (*strfunc_t)(const char *, char *);

/* Renaming different objects should not affect functionality */
typedef struct { char *realpath; } rpv_t;
typedef struct { strfunc_t realpath; } rpf_t;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>

int
main(int argc, char *argv[])
{
  int verbose = 0;
  const char *p, *q;
  strfunc_t f;
  rpf_t rpf = { realpath };
  rpv_t rpv;
  char buf[PATH_MAX], cwd[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) getcwd(cwd, sizeof(cwd));
  if (verbose) {
    printf("Starting realpath test.\n");
    printf("cwd = %s\n", cwd);
  }

  /* Test traditional version with supplied buffer */
  p = realpath(".", buf);
  assert(p && "realpath(path, buf) returned NULL");
  if (verbose) printf("realpath(path, buf) supported.\n");

  /* Test with direct function call */
  q = realpath(".", NULL);
  assert(q && "realpath(path, NULL) returned NULL");
  assert (!strcmp(q, p) && "realpath(path, NULL) miscompared");
  if (verbose) printf("realpath(path, NULL) supported.\n");
  free((void*)q);

  /* Test with name (reference) only */
  f = realpath;
  q = f(".", NULL);
  assert(q && "realpath, f(path, NULL) returned NULL");
  assert (!strcmp(q, p) && "realpath, f(path, NULL) miscompared");
  if (verbose) printf("f = realpath, f(path, NULL) supported.\n");
  free((void*)q);

  /* Test with function macro disabler */
  q = (realpath)(".", NULL);
  assert(q && "(realpath)(path, NULL) returned NULL");
  assert (!strcmp(q, p) && "(realpath)(path, NULL) miscompared");
  if (verbose) printf("(realpath)(path, NULL) supported.\n");
  free((void*)q);

  /* Test with same-named fields */
  rpv.realpath = rpf.realpath(".", NULL);
  assert(rpv.realpath && "rpf.realpath(path, NULL) returned NULL");
  assert (!strcmp(rpv.realpath, p) && "rpf.realpath(path, NULL) miscompared");
  if (verbose) printf("rpv.realpath = rpf.realpath(path, NULL) supported.\n");
  free((void*)rpv.realpath);

  printf("realpath test succeeded.\n");
  return 0;
}
