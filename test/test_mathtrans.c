/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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
#include <math.h>
#include <stdio.h>
#include <string.h>

static int
check_dbl(const char *name, double x, double y, double lim, int verbose)
{
  int ret;
  double err;

  err = fabs((x - y) / y);
  ret = err > lim;

  if (ret || (verbose && err > 0.0)) {
    printf("  %serror in %s() %.16f = %.20e\n",
           ret ? "*** " : "  ", name, x, err);
  }
  return ret;
}

static const double test_sincos = 0.9;
static const float test_sincosf = 0.9;
static const double test_exp10 = 1.5;
static const float test_exp10f = 1.5;

static const double test_sincos_sin = 0.7833269096274833;
static const double test_sincos_cos = 0.6216099682706645;
static const float test_sincosf_sin = 0.7833269238471985;
static const float test_sincosf_cos = 0.6216099858283997;

static const double test_exp10_res = 31.6227766016837926;
static const float test_exp10f_res = 31.6227760314941406;

static const double maxerr_dbl = 1E-15;
static const double maxerr_flt = 2E-7;

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0;
  char *progname = basename(argv[0]);
  double double1, double2;
  float float1, float2;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", progname);

  __sincos(test_sincos, &double1, &double2);
  if (verbose) {
    printf("  __sincos(%.16f)  = %.16f, %.16f\n",
           test_sincos, double1, double2);
  }
  ret |= check_dbl("__sincos", double1, test_sincos_sin, maxerr_dbl, verbose);
  ret |= check_dbl("__sincos", double2, test_sincos_cos, maxerr_dbl, verbose);

  __sincosf(test_sincos, &float1, &float2);
  if (verbose) {
    printf("  __sincosf(%.16f) = %.16f, %.16f\n", test_sincosf, float1, float2);
  }
  ret |= check_dbl("__sincosf", float1, test_sincosf_sin, maxerr_flt, verbose);
  ret |= check_dbl("__sincosf", float2, test_sincosf_cos, maxerr_flt, verbose);

  double1 = __exp10(test_exp10);
  if (verbose) {
    printf("  __exp10(%.16f)  = %.16f\n", test_exp10, double1);
  }
  ret |= check_dbl("__exp10", double1, test_exp10_res, maxerr_dbl, verbose);

  float1 = __exp10f(test_exp10f);
  if (verbose) {
    printf("  __exp10f(%.16f) = %.16f\n", test_exp10f, float1);
  }
  ret |= check_dbl("__exp10f", float1, test_exp10f_res, maxerr_flt, verbose);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
