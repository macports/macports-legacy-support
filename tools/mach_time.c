/*
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

/* This is a simple too to report info about mach_time scaling */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <mach/mach_time.h>

#include <sys/sysctl.h>

#if defined(__ppc__)
#define ARCH "ppc"
#elif defined(__ppc64__)
#define ARCH "ppc64"
#elif defined(__i386__)
#define ARCH "i386"
#elif defined(__x86_64__)
#define ARCH "x86_64"
#elif defined(__arm__)
#define ARCH "arm"
#elif defined(__arm64__) || defined(__aarch64__)
#define ARCH "arm64"
#else
#define ARCH "unknown"
#endif

#define SYSCTL_OSVER_CLASS CTL_KERN
#define SYSCTL_OSVER_ITEM  KERN_OSRELEASE

/* sysctl to check whether we're running natively (not Rosetta 1) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* sysctl to check whether we're running in Rosetta 2 */
#define SYSCTL_TRANSLATED "sysctl.proc_translated"

#define EXTRA_SHIFT 2
#define HIGH_SHIFT (32 + EXTRA_SHIFT)
#define HIGH_BITS (64 - HIGH_SHIFT)
#define NUMERATOR_MASK (~0U << HIGH_BITS)

static char osver[256];

static int
get_osver(void)
{
  int mib[] = {SYSCTL_OSVER_CLASS, SYSCTL_OSVER_ITEM};
  int miblen = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(osver);

  if (sysctl(mib, miblen, osver, &len, NULL, 0)) return -1;
  if (len <= 0 || len >= (ssize_t) sizeof(osver)) return -1;
  osver[len] = '\0';
  if (osver[len - 1] == '\n') osver[len - 1] = '\0';
  return 0;
}

/* Test whether running under Rosetta */
/* -1 no, 1 Rosetta 1, 2 Rosetta 2 */
static int
check_rosetta(void)
{
  int native, translated;
  size_t native_sz = sizeof(native);
  size_t translated_sz = sizeof(translated);

  /* Check for Rosetta 1 */
  if (sysctlbyname(SYSCTL_NATIVE, &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    return -1;
  }
  if (!native) return 1;

  /* If "native", check for Rosetta 2 */
  if (sysctlbyname(SYSCTL_TRANSLATED, &translated, &translated_sz,
                   NULL, 0) < 0) {
    /* If sysctl failed, must be really native. */
    return -1;
  }
  return translated ? 2 : -1;
}

int
main(int argc, char *argv[])
{
  int err, tbinfo_err;
  const char *rosetta;
  unsigned long val;
  char *cp;
  mach_timebase_info_data_t tbinfo;
  double scale, dfscale;
  double limit;
  uint64_t scale_ns;
  double fscale_ns;
  double error_ns, dferror_ns, serror_ns;
  const char *approxstr, *err_units;

  if (argc >= 3) {
    val = strtoul(argv[1], &cp, 0);
    if (*cp) {
      printf("Bad numerator\n");
      return 1;
    }
    tbinfo.numer = val;
    val = strtoul(argv[2], &cp, 0);
    if (*cp) {
      printf("Bad denominator\n");
      return 1;
    }
    tbinfo.denom = val;
  } else {
    err = get_osver();
    switch (check_rosetta()) {
      case -1: rosetta = "native"; break;
      case 1: rosetta = "Rosetta 1"; break;
      case 2: rosetta = "Rosetta 2"; break;
      default: rosetta = "???"; break;
    }
    printf("OS is Darwin %s, CPU is %s (%s)\n",
           err ? "???" : osver, ARCH, rosetta);

    tbinfo_err = mach_timebase_info(&tbinfo);
    if (tbinfo_err != KERN_SUCCESS) {
      printf("Unable to obtain timebase rate, err = %d\n", tbinfo_err);
      return 0;
    }
  }

  scale = (double) tbinfo.numer / tbinfo.denom;
  dfscale = (double) (tbinfo.numer / tbinfo.denom);
  approxstr = (tbinfo.numer % tbinfo.denom) ? "~" : "";
  limit = ldexp(1.0, 64) / tbinfo.denom / 1E9;

  if (!(tbinfo.numer & NUMERATOR_MASK)) {
    scale_ns = (((uint64_t) tbinfo.numer << HIGH_SHIFT) + (tbinfo.denom >> 1))
               / tbinfo.denom;
  } else {
    scale_ns = ((((uint64_t) tbinfo.numer << 32) + (tbinfo.denom >> 1))
                / tbinfo.denom) << EXTRA_SHIFT;
  }
  fscale_ns = (double) (scale_ns >> 32) + ldexp(scale_ns & 0xFFFFFFFF,-32);
  error_ns = (ldexp(fscale_ns, -EXTRA_SHIFT) - scale) / scale;
  dferror_ns = (dfscale - scale) / scale;
  if (fabs(error_ns) >=1E-6) {
    serror_ns = error_ns * 1E6; err_units = "ppm";
  } else {
    serror_ns = error_ns * 1E9; err_units = "ppb";
  }

  printf("Mach absolute time multiplier (%db/%db) = %u/%u %s= %f\n",
         (int) sizeof(tbinfo.numer) * CHAR_BIT,
         (int) sizeof(tbinfo.denom) * CHAR_BIT,
         tbinfo.numer, tbinfo.denom,
         approxstr, scale);
  printf("Mach absolute time frequency = %.3f MHz\n", 1000.0 / scale);
  if (limit >= 86400) {
    printf("Mach multiply-first scaling overflow at %.1f days, %.2f years\n",
           limit / 86400, limit / (86400 * 365.25));
  } else {
    printf("Mach multiply-first scaling overflow at %.2f minutes, %.2f hours\n",
           limit / 60, limit / 3600);
  }
  printf("Mach divide-first scaling error = %.3f ppm\n", dferror_ns * 1E6);
  printf("Nanosecond 64-bit 30b.34b multiplier = 0x%08X:%08X %s= %f,"
         " relative error = %.6f %s\n",
         (uint32_t) (scale_ns >> 32), (uint32_t) (scale_ns & 0xFFFFFFFF),
         approxstr, ldexp(fscale_ns, -EXTRA_SHIFT), serror_ns, err_units);
  return 0;
}
