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

/*
 * This is an investigative tool for capturing runs of clock samples and
 * reporting a histogram of the deltas, or of diffs/deltas from an interleaved
 * capture of a test clock vs. mach_absolute_time.
 *
 * This tool is intended to be built without legacy-support, but can
 * optionally load the legacy-support library from either the "system"
 * location or the relative build-tree location.  In the former case,
 * the default "/opt/local" prefix is assumed, unless the MPPREFIX
 * definition is overridden.
 */

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mach/mach_time.h>

#include <sys/param.h>
#include <sys/time.h>

/*
 * Allow builds with <10.12 SDK (and no legacy-support headers).
 * This a simplified version of the relevant portion of the 10.12 time.h.
 */
#ifndef CLOCK_REALTIME
typedef enum {
_CLOCK_REALTIME = 0,
#define CLOCK_REALTIME _CLOCK_REALTIME
_CLOCK_MONOTONIC = 6,
#define CLOCK_MONOTONIC _CLOCK_MONOTONIC
_CLOCK_MONOTONIC_RAW = 4,
#define CLOCK_MONOTONIC_RAW _CLOCK_MONOTONIC_RAW
_CLOCK_MONOTONIC_RAW_APPROX = 5,
#define CLOCK_MONOTONIC_RAW_APPROX _CLOCK_MONOTONIC_RAW_APPROX
_CLOCK_UPTIME_RAW = 8,
#define CLOCK_UPTIME_RAW _CLOCK_UPTIME_RAW
_CLOCK_UPTIME_RAW_APPROX = 9,
#define CLOCK_UPTIME_RAW_APPROX _CLOCK_UPTIME_RAW_APPROX
} clockid_t;
#endif /* CLOCK_REALTIME undef */

/* RTLD_FIRST is unavailable on 10.4 - make it ignored. */
#ifndef RTLD_FIRST
#define RTLD_FIRST 0
#endif

#define DEF_NUM_DIFFS 1000
#define DEF_SLEEP_MS  5

#define REF_CLOCK_IDX clock_idx_mach_absolute
#define DEF_CLOCK_IDX REF_CLOCK_IDX

#define MIN_DIFFS 3
#define MAX_DIFFS 1000000

#define MIN_SLEEP 1
#define MAX_SLEEP 1000

#ifndef MPPREFIX
#define MPPREFIX "/opt/local"
#endif
#define LIBDIR "lib"
#define LSLIB "libMacportsLegacySupport.dylib"
#define MPLSLIB MPPREFIX "/" LIBDIR "/" LSLIB
#define LOCALLSLIB "../" LIBDIR "/" LSLIB

typedef uint64_t mach_time_t;
typedef struct timeval timeval_t;
typedef struct timespec timespec_t;
typedef uint64_t ns_time_t;
typedef int64_t sns_time_t;

#define BILLION64  1000000000ULL
#define UL (unsigned long)
#define ULL (unsigned long long)
#define LL  (long long)

typedef mach_time_t (mach_fn_t)(void);
typedef int (timeofday_fn_t)(timeval_t *, void *);
typedef int (gettime_fn_t)(clockid_t, timespec_t *);
typedef ns_time_t (gettime_ns_fn_t)(clockid_t);

/*
 * The macros setting up various tables related to clock types are based on
 * macros in test_clocks.c, but creating a common header isn't worth the
 * complexity.
 */

/* List of clock types */
/* #define CLOCK_TYPE(name,valtype) */
#define CLOCK_TYPES \
  CLOCK_TYPE(mach,mach_time_t) \
  CLOCK_TYPE(timeofday,timeval_t) \
  CLOCK_TYPE(gettime,timespec_t) \
  CLOCK_TYPE(gettime_ns,ns_time_t) \

/* Clock type enums */
#define CLOCK_TYPE(name,buf) clock_type_##name,
typedef enum clock_type {
  CLOCK_TYPES
} clock_type_t;
#undef CLOCK_TYPE

/* List of non-process-specific clocks */
/* #define NP_CLOCK(name,type) */
#define NP_CLOCKS \
  NP_TOD_CLOCKS \
  NP_MACH_CLOCKS \
  NP_GETTIME_CLOCKS(gettime) \
  NP_GETTIME_CLOCKS(gettime_ns) \

/* Time of day clock */
#define NP_TOD_CLOCKS \
  NP_CLOCK(timeofday,timeofday) \

/* Mach clocks */
#define NP_MACH_CLOCKS \
  NP_CLOCK(absolute,mach) \
  NP_CLOCK(approximate,mach) \
  NP_CLOCK(continuous,mach) \
  NP_CLOCK(continuous_approximate,mach) \

/* Gettime clocks (for both flavors) */
#define NP_GETTIME_CLOCKS(type) \
  NP_CLOCK(REALTIME,type) \
  NP_CLOCK(MONOTONIC,type) \
  NP_CLOCK(MONOTONIC_RAW,type) \
  NP_CLOCK(MONOTONIC_RAW_APPROX,type) \
  NP_CLOCK(UPTIME_RAW,type) \
  NP_CLOCK(UPTIME_RAW_APPROX,type) \

#define CALLMAC(a,b,c) a##b(c)

/* Clock type codes */
#define CLOCK_IDX_timeofday(name) clock_idx_##name
#define CLOCK_IDX_mach(name) clock_idx_mach_##name
#define CLOCK_IDX_gettime(name) clock_idx_gettime_##name
#define CLOCK_IDX_gettime_ns(name) clock_idx_gettime_ns_##name
#define NP_CLOCK(name,type) \
  CALLMAC(CLOCK_IDX_,type,name),
typedef enum clock_idx {
  NP_CLOCKS
} clock_idx_t;
#undef NP_CLOCK
#undef CLOCK_IDX_timeofday
#undef CLOCK_IDX_mach
#undef CLOCK_IDX_gettime
#undef CLOCK_IDX_gettime_ns

#define NP_CLOCK(name,type) clock_type_##type,
static const clock_type_t clock_types[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

/* Clock function names */
#define CLOCK_FUNC_timeofday(name) "gettimeofday"
#define CLOCK_FUNC_mach(name) "mach_" #name "_time"
#define CLOCK_FUNC_gettime(name) "clock_gettime"
#define CLOCK_FUNC_gettime_ns(name) "clock_gettime_nsec_np"
#define NP_CLOCK(name,type) \
  CALLMAC(CLOCK_FUNC_,type,name),
static const char * const clock_func_names[] = {
  NP_CLOCKS
};
#undef NP_CLOCK
#undef CLOCK_FUNC_timeofday
#undef CLOCK_FUNC_mach
#undef CLOCK_FUNC_gettime
#undef CLOCK_FUNC_gettime_ns

/* Arguments for collector functions */
#define CLOCK_ARG_timeofday(name) 0
#define CLOCK_ARG_mach(name) 0
#define CLOCK_ARG_gettime(name) CLOCK_##name
#define CLOCK_ARG_gettime_ns(name) CLOCK_##name
#define NP_CLOCK(name,type) \
  CALLMAC(CLOCK_ARG_,type,name),
static const clockid_t clock_ids[] = {
  NP_CLOCKS
};
#undef NP_CLOCK
#undef CLOCK_ARG_timeofday
#undef CLOCK_ARG_mach
#undef CLOCK_ARG_gettime
#undef CLOCK_ARG_gettime_ns

/* Clock names */
#define CLOCK_NAME_timeofday(name) "timeofday"
#define CLOCK_NAME_mach(name) "mach_" #name "_time"
#define CLOCK_NAME_gettime(name) "CLOCK_" #name
#define CLOCK_NAME_gettime_ns(name) "CLOCK_" #name "_ns"
#define NP_CLOCK(name,type) \
  CALLMAC(CLOCK_NAME_,type,name),
static const char * const clock_names[] = {
  NP_CLOCKS
  NULL
};
#undef NP_CLOCK
#undef CLOCK_NAME_timeofday
#undef CLOCK_NAME_mach
#undef CLOCK_NAME_gettime
#undef CLOCK_NAME_gettime_ns

/* Union of clock function pointers */
#define CLOCK_TYPE(name,valtype) name##_fn_t *name;
typedef union clock_funcp_u {
  CLOCK_TYPES
} clock_funcp_t;
#undef CLOCK_TYPE

/* Union of clock value pointers */
#define CLOCK_TYPE(name,valtype) valtype *name;
typedef union clock_bufp_u {
  CLOCK_TYPES
} clock_bufp_t;
#undef CLOCK_TYPE

/* Struct for histogram entry */
typedef struct histent_s {
  sns_time_t diff;
  int count;
} histent_t;

/* Generic structure for clock info */
typedef struct clock_info_s {
  clock_idx_t idx;
  clock_type_t type;
  long numdiffs;
  long sleepus;
  clock_funcp_t f;
  clockid_t clkid;
  clock_bufp_t b;
  clock_bufp_t be;
  ns_time_t *nsbuf;
  ns_time_t *nsbufe;
  histent_t *hbuf;
  histent_t *hbufe;
  sns_time_t mean_diff;
} clock_info_t;

/* Mach clock scale factors */
static mach_timebase_info_data_t tbinfo;
static long double mach2nanos, mach2usecs;

/* Bit bucket for cache warmup calls */
static volatile union scratch_u {
  mach_time_t mach;
  timeval_t timeval;
  timespec_t timespec;
  ns_time_t ns_time;
} time_scratch;

/* Set up initial parameters in clock_info */
static void
setup_info(clock_info_t *ci, clock_idx_t idx, long numdiffs, long sleepms)
{
  ci->idx = idx;
  ci->type = clock_types[idx];
  ci->numdiffs = numdiffs;
  ci->sleepus = sleepms * 1000;
  ci->clkid = clock_ids[idx];
}

/* Conversions from different time formats to nanoseconds */

static ns_time_t
tv2nsec(timeval_t *tv)
{
  return tv->tv_sec * BILLION64 + tv->tv_usec * 1000;
}

static ns_time_t
mt2nsec(mach_time_t mach_time)
{
  return mach_time * mach2nanos;
}

static ns_time_t
ts2nsec(timespec_t *ts)
{
  return ts->tv_sec * BILLION64 + ts->tv_nsec;
}

/* Plus a microsecond conversion */
static useconds_t
mt2usec(mach_time_t mach_time)
{
  return mach_time * mach2usecs;
}

/* Mach time in microseconds */
static useconds_t
mach_usec(void)
{
  return mt2usec(mach_absolute_time());
}

/* Version of usleep() that defends against signals (just paranoia) */
static void
usleepx(useconds_t usecs)
{
  useconds_t now = mach_usec();
  useconds_t target = now + usecs;

  do {
    (void) usleep(target - now);
  } while ((now = mach_usec()) < target);
}

static int
setup_mach(int verbose)
{
  int err;

  if ((err = mach_timebase_info(&tbinfo))) return err;
  mach2nanos = (long double) tbinfo.numer / tbinfo.denom;
  mach2usecs = mach2nanos / 1000.0;
  if (verbose) {
    printf("  Scale for mach_time (nanoseconds per unit) is %u/%u = %.3f\n",
           tbinfo.numer, tbinfo.denom, (double) mach2nanos);
  }
  return 0;
}

static void *
load_lib(int legacy, char *progname, int verbose)
{
  char *progdir;
  char lslib[PATH_MAX], lsreal[PATH_MAX];
  const char *libpath;
  void *libhandle = NULL;

  if (legacy > 0) {
    libpath = MPLSLIB;
  } else {
    progdir = dirname(progname);
    (void) snprintf(lslib, sizeof(lslib), "%s/" LOCALLSLIB, progdir);
    if (!(libpath = realpath(lslib, lsreal))) {
      fprintf(stderr, "Unable to resolve library path '%s': %s\n",
              lslib, strerror(errno));
      return NULL;
    }
  }
  if (!(libhandle = dlopen(libpath, RTLD_FIRST))) {
    fprintf(stderr, "Unable to open library: %s\n", dlerror());
    return NULL;
  }
  if (verbose) {
    printf("    Loaded %s, handle = 0x%0*lX\n", libpath,
           (int) sizeof(void *) * 2, UL libhandle);
  }
  return libhandle;
}

static void
close_lib(void **libhandle)
{
  if (*libhandle && dlclose(*libhandle)) {
    fprintf(stderr, "Unable to close library: %s\n", dlerror());
  }
  *libhandle = NULL;
}

static void *
clock_lookup(const char *name, void **handlep)
{
  void *adr;

  /* Try extra library first, then general */
  if (*handlep && (adr = dlsym(*handlep, name))) return adr;
  *handlep = RTLD_NEXT;
  return dlsym(RTLD_NEXT, name);
}

static const char *
clock_find(clock_info_t *ci, void *libhandle, int verbose)
{
  const char *name = clock_func_names[ci->idx];
  void *func;

  if (!(func = clock_lookup(name, &libhandle))) return name;

  switch (ci->type) {
    #define CLOCK_TYPE(name,valtyp) case clock_type_##name: \
      ci->f.name = func; \
      break;
      CLOCK_TYPES
    #undef CLOCK_TYPE
  }

  if (verbose) {
    if (libhandle == RTLD_NEXT) {
      printf("    Located %s in library handle RTLD_NEXT\n", name);
    } else {
      printf("    Located %s in library handle 0x%0*lX\n", name,
             (int) sizeof(void *) * 2, UL libhandle);
    }
  }

  return NULL;
}

static int
clock_alloc(clock_info_t *ci)
{
  void *bufp;
  int err;

  switch (ci->type) {
    #define CLOCK_TYPE(name,valtyp) case clock_type_##name: \
      bufp = ci->b.name = calloc(sizeof(valtyp), ci->numdiffs + 1); \
      ci->be.name = bufp ? ci->b.name + ci->numdiffs : NULL; \
      break;
      CLOCK_TYPES
    #undef CLOCK_TYPE
  }
  if (!bufp) return -1;

  if (!(ci->nsbuf = calloc(sizeof(ns_time_t), ci->numdiffs + 1))) {
    err = errno;
    free(bufp);
    errno = err;
    return -1;
  }
  ci->nsbufe = ci->nsbuf + ci->numdiffs;

  if (!(ci->hbuf = calloc(sizeof(histent_t), ci->numdiffs))) {
    err = errno;
    free(ci->nsbuf);
    free(bufp);
    errno = err;
    return -1;
  }

  return 0;
}

static void
clock_free(clock_info_t *ci)
{
  void *bufp;

  switch (ci->type) {
    #define CLOCK_TYPE(name,valtyp) case clock_type_##name: \
      bufp = ci->b.name; ci->b.name = ci->be.name = NULL; \
      break;
      CLOCK_TYPES
    #undef CLOCK_TYPE
  }
  free(ci->hbuf); ci->hbuf = NULL;
  free(ci->nsbuf); ci->nsbuf = ci->nsbufe = NULL;
  free(bufp);
}

/* Universal collector/converter function */
static int
clock_collect(clock_info_t *ci)
{
  clock_funcp_t funcp = ci->f;
  clockid_t clkid = ci->clkid;
  int ret;
  clock_bufp_t bufp = ci->b, cbufp = ci->b;
  clock_bufp_t bufe = ci->be;
  ns_time_t *nbp = ci->nsbuf;

  usleepx(ci->sleepus);

  #define CLOCK_CALL_mach(type) \
    time_scratch.mach = (*funcp.type)(); \
    while (bufp.type <= bufe.type) { \
      *bufp.type++ = (*funcp.type)(); \
    } \
    while (cbufp.type <= bufe.type) { \
      *nbp++ = mt2nsec(*cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_timeofday(type) \
    (void) (*funcp.type)(bufp.type, NULL); \
    while (bufp.type <= bufe.type) { \
      if ((ret = (*funcp.type)(bufp.type++, NULL))) return ret; \
    } \
    while (cbufp.type <= bufe.type) { \
      *nbp++ = tv2nsec(cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_gettime(type) \
    (void) (*funcp.type)(clkid, bufp.type); \
    while (bufp.type <= bufe.type) { \
      if ((ret = (*funcp.type)(clkid, bufp.type++))) return ret; \
    } \
    while (cbufp.type <= bufe.type) { \
      *nbp++ = ts2nsec(cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_gettime_ns(type) \
    time_scratch.ns_time = (*funcp.type)(clkid); \
    while (bufp.type <= bufe.type) { \
      if (!(*bufp.type++ = (*funcp.type)(clkid))) return -1; \
    } \
    while (cbufp.type <= bufe.type) { \
      *nbp++ = *cbufp.type++; \
    } \
    break;

  switch (ci->type) {
    #define CLOCK_TYPE(name,valtyp) case clock_type_##name: \
      CALLMAC(CLOCK_CALL_,name,name)
      CLOCK_TYPES
    #undef CLOCK_TYPE
  }

  #undef CLOCK_CALL_timeofday
  #undef CLOCK_CALL_mach
  #undef CLOCK_CALL_gettime
  #undef CLOCK_CALL_gettime_ns

  return 0;
}

/* Universal compare collector/converter function */
static int
clock_compare(clock_info_t *ci, clock_info_t *rci)
{
  clock_funcp_t funcp = ci->f;
  clockid_t clkid = ci->clkid;
  int ret;
  clock_bufp_t bufp = ci->b, cbufp = ci->b;
  clock_bufp_t bufe = ci->be;
  ns_time_t *nbp = ci->nsbuf;
  mach_time_t *mtrp = rci->b.mach, *rmtrp = rci->b.mach;
  mach_time_t *mtpe = rci->be.mach;
  ns_time_t *rnbp = rci->nsbuf;

  assert(rci->idx == clock_idx_mach_absolute && "Unexpected reference clock");

  usleepx(ci->sleepus);

  #define CLOCK_CALL_mach(type) \
    time_scratch.mach = mach_absolute_time(); \
    time_scratch.mach = (*funcp.type)(); \
    *mtrp++ = mach_absolute_time(); \
    while (bufp.type < bufe.type) { \
      *bufp.type++ = (*funcp.type)(); \
      *mtrp++ = mach_absolute_time(); \
    } \
    while (cbufp.type < bufe.type) { \
      *nbp++ = mt2nsec(*cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_timeofday(type) \
    time_scratch.mach = mach_absolute_time(); \
    (void) (*funcp.type)(bufp.type, NULL); \
    *mtrp++ = mach_absolute_time(); \
    while (bufp.type < bufe.type) { \
      if ((ret = (*funcp.type)(bufp.type++, NULL))) return ret; \
      *mtrp++ = mach_absolute_time(); \
    } \
    while (cbufp.type < bufe.type) { \
      *nbp++ = tv2nsec(cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_gettime(type) \
    time_scratch.mach = mach_absolute_time(); \
    (void) (*funcp.type)(clkid, bufp.type); \
    *mtrp++ = mach_absolute_time(); \
    while (bufp.type < bufe.type) { \
      if ((ret = (*funcp.type)(clkid, bufp.type++))) return ret; \
      *mtrp++ = mach_absolute_time(); \
    } \
    while (cbufp.type < bufe.type) { \
      *nbp++ = ts2nsec(cbufp.type++); \
    } \
    break;
  #define CLOCK_CALL_gettime_ns(type) \
    time_scratch.mach = mach_absolute_time(); \
    time_scratch.ns_time = (*funcp.type)(clkid); \
    *mtrp++ = mach_absolute_time(); \
    while (bufp.type < bufe.type) { \
      if (!(*bufp.type++ = (*funcp.type)(clkid))) return -1; \
      *mtrp++ = mach_absolute_time(); \
    } \
    while (cbufp.type < bufe.type) { \
      *nbp++ = *cbufp.type++; \
    } \
    break;

  switch (ci->type) {
    #define CLOCK_TYPE(name,valtyp) case clock_type_##name: \
      CALLMAC(CLOCK_CALL_,name,name)
      CLOCK_TYPES
    #undef CLOCK_TYPE
  }

  #undef CLOCK_CALL_timeofday
  #undef CLOCK_CALL_mach
  #undef CLOCK_CALL_gettime
  #undef CLOCK_CALL_gettime_ns

  while (rmtrp <= mtpe) {
    *rnbp++ = mt2nsec(*rmtrp++);
  }
  return 0;
}

/* Dump all clock samples */
static void
clock_dump_ns(clock_info_t *ci, int quiet)
{
  int index;
  ns_time_t cur;
  sns_time_t last = quiet > 2 ? ci->nsbuf[0] : -1;

  if (!quiet) printf("Samples of '%s':\n", clock_names[ci->idx]);
  for (index = 0; index <= ci->numdiffs; ++index) {
    cur = ci->nsbuf[index];
    if (quiet > 2) {
      printf("%7d %llu %4d\n", index, ULL cur, (int) (cur - last));
    } else if (last < 0) {
      printf("%7d: %llu\n", index, ULL cur);
    } else {
      printf("%7d: %llu (%d)\n", index, ULL cur, (int) (cur - last));
    }
    last = cur;
  }
}

/* Dump all interleaved samples */
static void
clock_dump_dual_ns(clock_info_t *ci, clock_info_t *rci, int quiet)
{
  int numdiffs = ci->numdiffs;
  mach_time_t *rmp = rci->b.mach;
  sns_time_t ref_mean, tst_mean, mean_diff;
  int index;
  ns_time_t cur, next, tstcur;
  sns_time_t last = quiet > 2 ? rci->nsbuf[0] : -1, mean, diff;

  ref_mean = mt2nsec(rmp[0] + rmp[1] + rmp[numdiffs-1] + rmp[numdiffs]) / 4;
  tst_mean = (ci->nsbuf[0] + ci->nsbuf[numdiffs-1]) / 2;
  mean_diff = tst_mean - ref_mean;

  if (!quiet) {
    printf("Samples (ns) of '%s', interleaved with '%s':\n",
           clock_names[ci->idx], clock_names[rci->idx]);
    printf("  Ref mean = %lld, Test mean = %lld, Diff = %lld\n",
           LL ref_mean, LL tst_mean, LL mean_diff);
  }
  for (index = 0; index < numdiffs; ++index) {
    cur = rci->nsbuf[index];
    if (quiet <= 2) {
      if (last < 0) {
        printf("%7d: %llu\n", index, ULL cur);
      } else {
        printf("%7d: %llu (%d)\n", index, ULL cur, (int) (cur - last));
      }
    }
    next = rci->nsbuf[index+1];
    mean = (cur + next) / 2;
    tstcur = ci->nsbuf[index];
    diff = tstcur - mean;
    if (quiet <= 2) {
      printf("         %llu (%lld +mean_diff %+lld)\n",
             ULL tstcur, LL mean, LL (diff - mean_diff));
    } else {
      printf("%7d %llu %4d %llu %lld %+4lld\n",
             index, ULL cur, (int) (cur - last),
             ULL tstcur, LL mean, LL (diff - mean_diff));
    }
    last = cur;
  }
  cur = rci->nsbuf[index];
  printf(quiet <= 2 ? "%7d: %llu (%d)\n" : "%7d %llu %4d\n",
         index, ULL cur, (int) (cur - last));
}

/* Generate deltas for histogram */
static void
gen_deltas(clock_info_t *ci)
{
  ns_time_t *nsp = ci->nsbuf;
  histent_t *hp = ci->hbuf;
  sns_time_t last, cur;

  last = *nsp++;
  while (nsp <= ci->nsbufe) {
    cur = *nsp++;
    hp->diff = cur - last; hp++->count = 0;
    last = cur;
  }
}

/* Generate offsets for histogram */
static void
gen_offsets(clock_info_t *ci, clock_info_t *rci)
{
  int numdiffs = ci->numdiffs;
  mach_time_t *rmp = rci->b.mach;
  sns_time_t ref_mean, tst_mean;
  int index;
  ns_time_t cur, next, tstcur;
  sns_time_t last = -1, mean, diff;
  histent_t *hp = ci->hbuf;

  ref_mean = mt2nsec(rmp[0] + rmp[1] + rmp[numdiffs-1] + rmp[numdiffs]) / 4;
  tst_mean = (ci->nsbuf[0] + ci->nsbuf[numdiffs-1]) / 2;
  ci->mean_diff = tst_mean - ref_mean;

  for (index = 0; index < numdiffs; ++index) {
    cur = rci->nsbuf[index];
    next = rci->nsbuf[index+1];
    mean = (cur + next) / 2;
    tstcur = ci->nsbuf[index];
    diff = tstcur - mean;
    hp->diff = diff - ci->mean_diff; hp++->count = 0;
    last = cur;
  }
}

/* Sort deltas */
static int
hcomp_diff(const void *h1v, const void *h2v)
{
  const histent_t *h1 = (const histent_t *) h1v;
  const histent_t *h2 = (const histent_t *) h2v;

  if (h1->diff == h2->diff) return 0;
  return h1->diff < h2->diff ? -1 : 1;
}

static void
hsort_diff(clock_info_t *ci)
{
  qsort(ci->hbuf, ci->numdiffs, sizeof(histent_t), hcomp_diff);
}

/* Compress runs of identical diffs */
static void
hcoll_diff(clock_info_t *ci)
{
  histent_t *hpi = ci->hbuf, *hpo = ci->hbuf;
  histent_t *hpe = ci->hbuf + ci->numdiffs;

  while (hpi < hpe) {
    if (hpi->diff == hpo->diff) {
      ++hpo->count; ++hpi;
    } else {
      *++hpo = *hpi;
    }
  }
  ci->hbufe = hpo + 1;
}

/* Generate histogram */
static void
gen_hist(clock_info_t *ci)
{
  gen_deltas(ci);
  hsort_diff(ci);
  hcoll_diff(ci);
}

/* Generate comparison histogram */
static void
gen_hist2(clock_info_t *ci, clock_info_t *rci)
{
  gen_offsets(ci, rci);
  hsort_diff(ci);
  hcoll_diff(ci);
}

/* Print histogram */
static void
dump_hist(clock_info_t *ci, int diffmode, int quiet)
{
  const histent_t *hp = ci->hbuf;
  const char *type = diffmode ? "diff" : "delta";

  if (!quiet) {
    printf("Histogram of ns %ss for '%s' (%s: count):\n",
           type, clock_names[ci->idx], type);
  }
  while (hp < ci->hbufe) {
    printf("%5d: %5d\n", (int) hp->diff, (int) hp->count);
    ++hp;
  }
}

/* Print dual histogram */
static void
dump_hist2(clock_info_t *ci, clock_info_t *rci, int quiet)
{
  const histent_t *hp = ci->hbuf, *rhp = rci->hbuf;

  if (!quiet) {
    printf("Histograms of ns diffs/deltas for '%s' vs '%s' (val: count)\n",
           clock_names[ci->idx], clock_names[rci->idx]);
    printf("  Diffs are relative to mean diff %lld\n",
           LL ci->mean_diff);
  }
  while (hp < ci->hbufe && rhp < rci->hbufe) {
    printf("%7d: %5d        %7d: %5d\n",
           (int) hp->diff, (int) hp->count, (int) rhp->diff, (int) rhp->count);
    ++hp; ++rhp;
  }
  while (hp < ci->hbufe) {
    printf("%7d: %5d\n", (int) hp->diff, (int) hp->count);
    ++hp;
  }
  while (rhp < rci->hbufe) {
    printf("                      %7d: %5d\n",
           (int) rhp->diff, (int) rhp->count);
    ++rhp;
  }
}

static long
getnum(const char *arg, const char *name, long minval, long maxval)
{
  long val;
  char *cp;

  val = strtol(arg, &cp, 0);
  if (*cp) {
    fprintf(stderr, "Bad %s argument.\n", name);
    exit(20);
  }
  if (val < minval || val > maxval) {
    fprintf(stderr, "Value %ld for %s out of range [%ld:%ld].\n",
            val, name, minval, maxval);
    exit(20);
  }
  return val;
}

static void
usage(FILE *fp, const char *name)
{
  fprintf(fp, "Usage is: %s [<opts>] [<clock> [<num diffs> [<sleep ms>]]]\n",
          name);
  fprintf(fp, "  Options:\n");
  fprintf(fp, "    -c:  Compare clock against mach_absolute_time\n");
  fprintf(fp, "    -h:  This text\n");
  fprintf(fp, "    -l:  List available clocks in this system\n");
  fprintf(fp, "    -L:  List defined clocks\n");
  fprintf(fp, "    -q:  Quiet (suppress headers)\n");
  fprintf(fp, "    -v:  Verbose output\n");
  fprintf(fp, "    -y:  Load system legacy-support library\n");
  fprintf(fp, "    -Y:  Load build-tree legacy-support library\n");
}

static void
list_clocks(int all, void *libhandle, int quiet)
{
  const char *cname, *fname;
  clock_idx_t clockidx = 0;
  void *handle;

  if (!quiet) {
    printf("%s clock names:\n", all ? "Defined" : "Available");
  }
  while (1) {
    fname = clock_func_names[clockidx];
    if (!(cname = clock_names[clockidx++])) break;
    if (!all) {
      handle = libhandle;
      if (!clock_lookup(fname, &handle)) continue;
    }
    printf("  %s\n", cname);
  }
}

int
main(int argc, char *argv[])
{
  int argn = 1;
  int compare = 0, help = 0, list = 0, quiet = 0, verbose = 0, legacy = 0;
  int err = 0;
  const char *cp;
  char chr;
  clock_idx_t clockidx = DEF_CLOCK_IDX;
  char *name = basename(argv[0]);
  void *libhandle = NULL;
  const char *clock_name = NULL;
  long numdiffs = DEF_NUM_DIFFS, sleepms = DEF_SLEEP_MS;
  clock_info_t tci = {0}, rci = {0};

  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 'c': compare = 1; break;
        case 'h': help = 1; break;
        case 'l': list = 1; break;
        case 'L': list = -1; break;
        case 'q': ++quiet; break;
        case 'v': ++verbose; break;
        case 'y': legacy = 1; break;
        case 'Y': legacy = -1; break;
      }
    }
    ++argn;
  }
  if (argn < argc) {
    clock_name = argv[argn];
    ++argn;
  }
  if (argn < argc) {
    numdiffs = getnum(argv[argn], "num_diffs", MIN_DIFFS, MAX_DIFFS);
    ++argn;
  }
  if (argn < argc) {
    sleepms = getnum(argv[argn], "sleep (ms)", MIN_SLEEP, MAX_SLEEP);
    ++argn;
  }

  if (help) usage(stdout, name);
  if (list < 0) list_clocks(1, NULL, quiet);
  if ((help && !list) || list < 0 ) return 0;

  if (legacy) {
    if (!(libhandle = load_lib(legacy, argv[0], verbose && !quiet))) return 10;
  }

  if (list > 0) {
    list_clocks(0, libhandle, quiet);
    return 0;
  }

  if (clock_name && strcmp(clock_name, ".")) {
    clockidx = 0;
    while ((cp = clock_names[clockidx])) {
      if (!strcmp(clock_name, cp)) break;
      ++clockidx;
    }
    if (!cp) {
      fprintf(stderr, "Unrecognized clock name '%s'\n", clock_name);
      return 20;
    }
  }

  setup_info(&tci, clockidx, numdiffs, sleepms);
  setup_info(&rci, REF_CLOCK_IDX, numdiffs, sleepms);

  do {
    if ((err = setup_mach(verbose && !quiet))) {
      perror("Unable to get mach time scale");
      break;
    }
    if ((cp = clock_find(&tci, libhandle, verbose && !quiet))) {
      fprintf(stderr, "Function '%s' is unavailable\n", cp);
      err = 10; break;
    }
    if (compare && (cp = clock_find(&rci, libhandle, verbose && !quiet))) {
      fprintf(stderr, "Function '%s' is unavailable\n", cp);
      err = 10; break;
    }

    if ((err = clock_alloc(&tci))) {
      perror("Unable to allocate sample buffers");
      break;
    }
    if (compare && (err = clock_alloc(&rci))) {
      perror("Unable to allocate reference buffers");
      break;
    }

    if (!compare) {
      if ((err = clock_collect(&tci))) {
        perror("Clock collection failed");
        break;
      }
      if (verbose >= 2) clock_dump_ns(&tci, quiet);
      gen_hist(&tci);
      if (quiet < 2) dump_hist(&tci, 0, quiet);
    } else {
      if ((err = clock_compare(&tci, &rci))) {
        perror("Clock compare collection failed");
        break;
      }
      if (verbose >= 2) clock_dump_dual_ns(&tci, &rci, quiet);
      gen_hist(&rci);
      gen_hist2(&tci, &rci);
      if (quiet < 2) dump_hist2(&tci, &rci, quiet);
    }
  } while (0);

  clock_free(&rci);
  clock_free(&tci);
  close_lib(&libhandle);

  if (!quiet) printf("%s %s.\n", name, err ? "failed" : "completed");
  return err;
}
