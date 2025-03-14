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
 * This provides tests of the various clock functions.
 *
 * In general, we represent most time values in 64-bit nanoseconds, which,
 * even as a signed quantity, has a range of about 292 years.  Intermediate
 * calculations may be done in floating-point, but its range may not be
 * sufficient for absolute (1970-relative) values in nanoseconds.  We sometimes
 * use long doubles, but they're not always better than doubles.
 */

#include <errno.h>
#include <libgen.h>
#include <math.h>
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
 * To determine the number of samples we collect, we *very* generously
 * assume that we could collect one sample per CPU clock, and that the
 * clock rate is no more than 4GHz.  With 10000 samples, that means
 * we get more than two microseconds of data as a minimum, which is
 * enough to see at least two transitions of a microsecond-resolution
 * clock.  At the other extreme, if we assume that the lowest conceivable
 * collection rate is 5MHz, then the collection should take no more than
 * two milliseconds, which means we can do all our tests in well under a
 * second.
 */

#define MIN_READ_TIME_PS 250
#define MIN_CLOCK_RES_NS 10
#define NUM_DIFFS 10000
#define NUM_SAMPLES (NUM_DIFFS+1)

typedef uint64_t mach_time_t;
typedef struct timeval timeval_t;
typedef struct timespec timespec_t;
typedef uint64_t ns_time_t;
typedef int64_t sns_time_t;

#define MILLION    1000000U
#define BILLION    1000000000U
#define BILLION64  1000000000ULL
#define BILLIONDBL 1000000000.0

/* Parameters for collection sequence */
#define MAX_STEP_NS 700000    /* Maximum delta not considered a step */
#define MAX_STEP_TRIES 50     /* Maximum tries to avoid a step */
#define STD_SLEEP_US 1000     /* Standard sleep before collecting */
#define MAX_SLEEP_US 100000   /* Maximum sleep when retrying */

#define LL (long long)
#define ULL (unsigned long long)

typedef unsigned long pointer_int_t;

static mach_timebase_info_data_t tbinfo;
static long double mach2nanos, mach2usecs, mach2secs;
static ns_time_t mach_res;

/* Bit bucket for cache warmup calls */
static volatile union scratch_u {
  mach_time_t mach;
  timeval_t timeval;
  timespec_t timespec;
  ns_time_t ns_time;
} time_scratch;

/* Local errors */

/* #define ONE_ERROR(name,text) */
#define OUR_ERRORS \
  ONE_ERROR(badmicros,"Bad tv_usec value") \
  ONE_ERROR(badnanos,"Bad tv_nsec value") \
  ONE_ERROR(resolution,"Bad clock resolution") \
  ONE_ERROR(backstep,"Too many retries to avoid backstep") \
  ONE_ERROR(illbackstep,"Clock illegally stepped backward") \
  ONE_ERROR(zerostep,"Too many consecutive unchanged samples") \
  ONE_ERROR(step,"Too many retries to avoid step") \
  ONE_ERROR(noerrno,"Error with errno not set") \

#define ONE_ERROR(name,text) err_##name,
typedef enum errs { err_dummy,  /* Avoid zero */
  OUR_ERRORS
} errs_t;
#undef ONE_ERROR

#define ONE_ERROR(name,text) text,
static const char * const errnames[] = { NULL,
  OUR_ERRORS
};
#undef ONE_ERROR

static const char *
get_errstr(int err)
{
  if (err < 0) return errnames[-err];
  return strerror(err);
}

/*
 * Properties of various clocks
 *
 * Type of clock (mach, time-of-day, gettime, gettime_ns)
 * Maximum zero deltas allowed
 * Step adjustments allowed
 * Slewing from adjtime() allowed
 * Approximate
 */

/* List of clock types */
/* #define CLOCK_TYPE(name,buf) */
#define CLOCK_TYPES \
  CLOCK_TYPE(mach,mtnsbuf) \
  CLOCK_TYPE(timeofday,tvnsbuf) \
  CLOCK_TYPE(gettime,tsnsbuf) \
  CLOCK_TYPE(gettime_ns,tsnsbuf) \

#define CLOCK_TYPE(name,buf) clock_type_##name,
typedef enum clock_type {
  CLOCK_TYPES
} clock_type_t;
#undef CLOCK_TYPE

/* List of non-process-specific clocks */
/* #define NP_CLOCK(name,type,okstep,okadj,approx) */
#define NP_CLOCKS \
  NP_TOD_CLOCKS \
  NP_MACH_CLOCKS \
  NP_GETTIME_CLOCKS(gettime) \
  NP_GETTIME_CLOCKS(gettime_ns) \

/* Time of day clock */
#define NP_TOD_CLOCKS \
  NP_CLOCK(timeofday,timeofday,1,1,0) \

/* Mach clocks */
#define NP_MACH_CLOCKS \
  NP_CLOCK(absolute,mach,0,0,0) \
  NP_CLOCK(approximate,mach,0,0,1) \
  NP_CLOCK(continuous,mach,0,0,0) \
  NP_CLOCK(continuous_approximate,mach,0,0,1)

/* Gettime clocks (for both flavors) */
#define NP_GETTIME_CLOCKS(type) \
  NP_CLOCK(REALTIME,type,1,1,0) \
  NP_CLOCK(MONOTONIC,type,0,1,0) \
  NP_CLOCK(MONOTONIC_RAW,type,0,0,0) \
  NP_CLOCK(MONOTONIC_RAW_APPROX,type,0,0,1) \
  NP_CLOCK(UPTIME_RAW,type,0,0,0) \
  NP_CLOCK(UPTIME_RAW_APPROX,type,0,0,1) \

#define CALLMAC(a,b,c) a##b(c)
#define CONC(a,b) a##b

/* Clock type codes */
#define CLOCK_IDX_timeofday(name) clock_idx_##name
#define CLOCK_IDX_mach(name) clock_idx_mach_##name
#define CLOCK_IDX_gettime(name) clock_idx_gettime_##name
#define CLOCK_IDX_gettime_ns(name) clock_idx_gettime_ns_##name
#define NP_CLOCK(name,type,okstep,okadj,approx) \
  CALLMAC(CLOCK_IDX_,type,name),
typedef enum clock_idx {
  NP_CLOCKS
} clock_idx_t;
#undef NP_CLOCK
#undef CLOCK_IDX_timeofday
#undef CLOCK_IDX_mach
#undef CLOCK_IDX_gettime
#undef CLOCK_IDX_gettime_ns

/*
 * Duplicate enum to get max value without "no default" switch() warnings.
 * Some compilers need this to be cast to clock_idx_t to avoid warnings
 * on comparisons.
 */
#define CLOCK_IDX_timeofday(name) clock_xidx_##name
#define CLOCK_IDX_mach(name) clock_xidx_mach_##name
#define CLOCK_IDX_gettime(name) clock_xidx_gettime_##name
#define CLOCK_IDX_gettime_ns(name) clock_xidx_gettime_ns_##name
#define NP_CLOCK(name,type,okstep,okadj,approx) \
  CALLMAC(CLOCK_IDX_,type,name),
typedef enum clock_xidx {
  NP_CLOCKS
  clock_idx_max,
} clock_xidx_t;
#undef NP_CLOCK
#undef CLOCK_IDX_timeofday
#undef CLOCK_IDX_mach
#undef CLOCK_IDX_gettime
#undef CLOCK_IDX_gettime_ns

#define NP_CLOCK(name,type,okstep,okadj,approx) clock_type_##type,
static const clock_type_t clock_types[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx) okstep,
static const int clock_okstep[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx) approx,
static const int clock_approx[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

/* Arguments for collector functions */
#define CLOCK_ARG_timeofday(name) NULL
#define CLOCK_ARG_mach(name) mach_##name##_time
#define CLOCK_ARG_gettime(name) ((void *) CLOCK_##name)
#define CLOCK_ARG_gettime_ns(name) ((void *) CLOCK_##name)
#define NP_CLOCK(name,type,okstep,okadj,approx) \
  CALLMAC(CLOCK_ARG_,type,name),
static void * const clock_args[] = {
  NP_CLOCKS
};
#undef NP_CLOCK
#undef CLOCK_ARG_timeofday
#undef CLOCK_ARG_mach
#undef CLOCK_ARG_gettime
#undef CLOCK_ARG_gettime_ns

/* Names for messages */
#define CLOCK_NAME_timeofday(name) "gettimeofday"
#define CLOCK_NAME_mach(name) "mach_" #name "_time"
#define CLOCK_NAME_gettime(name) "CLOCK_" #name
#define CLOCK_NAME_gettime_ns(name) "CLOCK_" #name "_ns"
#define NP_CLOCK(name,type,okstep,okadj,approx) \
  CALLMAC(CLOCK_NAME_,type,name),
static const char * const clock_names[] = {
  NP_CLOCKS
};
#undef NP_CLOCK
#undef CLOCK_NAME_timeofday
#undef CLOCK_NAME_mach
#undef CLOCK_NAME_gettime
#undef CLOCK_NAME_gettime_ns

/* Struct for clock stats */
typedef struct cstats_s {
  ns_time_t min;
  ns_time_t max;
  double mean;
  double stddev;
  int maxsame;
} cstats_t;

/* Struct for info on clock comparisons */
typedef struct dstats_s {
  ns_time_t refdiff;
  ns_time_t refmean;
  ns_time_t testdiff;
  ns_time_t testmean;
  sns_time_t offset;
  double slope;
  double sqmin;
  double sqmax;
  double sqmean;
} dstats_t;

/* Struct for info on clock errors */
typedef struct errinfo_s {
  ns_time_t first;
  ns_time_t last;
  int index;
  ns_time_t prev;
  ns_time_t cur;
  ns_time_t nzmin;
  int badsame;
  int maxsame;
  int retries;
  int errnum;
  timeval_t badtv;
  timespec_t badts;
} errinfo_t;

/* Buffers for clock values */
static timeval_t tvbuf[NUM_SAMPLES];
static mach_time_t mtbuf[NUM_SAMPLES];
static timespec_t tsbuf[NUM_SAMPLES];

/* Buffers for nanosecondized versions */
static ns_time_t tvnsbuf[NUM_SAMPLES];
static ns_time_t mtnsbuf[NUM_SAMPLES];
static ns_time_t tsnsbuf[NUM_SAMPLES];

/* Additional buffers for mach time as a comparison basis */
static mach_time_t refbuf[NUM_SAMPLES];
static ns_time_t refnsbuf[NUM_SAMPLES];

/* Pointers to buffers, by clock type */
#define CLOCK_TYPE(name,buf) &buf[0],
static ns_time_t * const nsbufp[] = {
  CLOCK_TYPES
};
#undef CLOCK_TYPE

static ns_time_t clock_res[clock_idx_max];

static char progname[PATH_MAX];

static int
setup(int verbose)
{
  if (mach_timebase_info(&tbinfo)) {
    printf("Can't get mach_time scale\n");
    return 1;
  }
  mach2nanos = (long double) tbinfo.numer / tbinfo.denom;
  mach2usecs = mach2nanos / 1000.0;
  mach2secs = mach2nanos / BILLIONDBL;
  mach_res = (tbinfo.numer + tbinfo.denom - 1) / tbinfo.denom;
  if (verbose) {
    printf("  Scale for mach_time (nanoseconds per unit) is %u/%u = %.3f\n",
           tbinfo.numer, tbinfo.denom, (double) mach2nanos);
  }
  return 0;
}

/* Verify errors with bad clockid */
static int
check_invalid(void)
{
  int ret = 0, err;
  timespec_t ts;
  ns_time_t ns;

  errno = -err_noerrno;
  if ((err = clock_getres(-1, &ts)) != -1 || errno != EINVAL) {
    printf("  *** clock_getres(-1, ...) = %d, errno = %d (%s)\n",
           err, errno, get_errstr(errno));
    ret = 1;
  }
  errno = -err_noerrno;
  if ((err = clock_gettime(-1, &ts)) != -1 || errno != EINVAL) {
    printf("  *** clock_gettime(-1, ...) = %d, errno = %d (%s)\n",
           err, errno, get_errstr(errno));
    ret = 1;
  }
  errno = -err_noerrno;
  if ((ns = clock_gettime_nsec_np(-1)) || errno != EINVAL) {
    printf("  *** clock_gettime_nsec_np(-1) = %lld, errno = %d (%s)\n",
           (long long) ns, errno, get_errstr(errno));
    ret = 1;
  }
  return ret;
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

/* Version of usleep() that defends against signals */
static void
usleepx(useconds_t usecs)
{
  useconds_t now = mach_usec();
  useconds_t target = now + usecs;

  do {
    (void) usleep(target - now);
  } while ((now = mach_usec()) < target);
}

/*
 * Functions to collect samples of one clock
 *
 * For best measurement accuracy, these first collect samples in their native
 * format, then convert to nanoseconds separately.
 */

typedef mach_time_t (mach_time_fn_t)(void);

static int
collect_mach(void *arg, errinfo_t *ei)
{
  mach_time_fn_t *func = (mach_time_fn_t *) arg;
  mach_time_t *mtp = &mtbuf[0];
  ns_time_t *nsp = &mtnsbuf[0];

  (void) ei;

  time_scratch.mach = (*func)();  /* Warm up cache */

  while (mtp < &mtbuf[NUM_SAMPLES]) {
    *mtp++ = (*func)();
  }

  mtp = &mtbuf[0];
  while (nsp < &mtnsbuf[NUM_SAMPLES]) {
    *nsp++ = mt2nsec(*mtp++);
  }
  return 0;
}

static int
collect_timeofday(void *arg, errinfo_t *ei)
{
  (void) arg;
  int ret = 0;
  timeval_t *tvp = &tvbuf[0];
  ns_time_t *nsp = &tvnsbuf[0];

  (void) gettimeofday(tvp, NULL);  /* Warm up cache */

  while (tvp < &tvbuf[NUM_SAMPLES]) {
    if ((ret = gettimeofday(tvp++, NULL))) {
      ei->errnum = errno;
      return ret;
    }
  }

  tvp = &tvbuf[0];
  while (nsp < &tvnsbuf[NUM_SAMPLES]) {
    if ((unsigned int) tvp->tv_usec >= MILLION) {
      ei->errnum = -err_badmicros;
      ei->badtv = *tvp;
      ret = -1;
    }
    *nsp++ = tv2nsec(tvp++);
  }
  return ret;
}

static int
collect_gettime(void *arg, errinfo_t *ei)
{
  clockid_t clkid = (clockid_t) (pointer_int_t) arg;
  int ret = 0;
  timespec_t *tsp = &tsbuf[0];
  ns_time_t *nsp = &tsnsbuf[0];

  (void) clock_gettime(clkid, tsp);  /* Warm up cache */

  while (tsp < &tsbuf[NUM_SAMPLES]) {
    if ((ret = clock_gettime(clkid, tsp++))) {
      ei->errnum = errno;
      return ret;
    }
  }

  tsp = &tsbuf[0];
  while (nsp < &tsnsbuf[NUM_SAMPLES]) {
    if ((unsigned int) tsp->tv_nsec >= BILLION) {
      ei->errnum = -err_badnanos;
      ei->badts = *tsp;
      ret = -1;
    }
    *nsp++ = ts2nsec(tsp++);
  }
  return ret;
}

static int
collect_gettime_ns(void *arg, errinfo_t *ei)
{
  clockid_t clkid = (clockid_t) (pointer_int_t) arg;
  ns_time_t *nsp = &tsnsbuf[0];

  time_scratch.ns_time = clock_gettime_nsec_np(clkid);  /* Warm up cache */

  while (nsp < &tsnsbuf[NUM_SAMPLES]) {
    if (!(*nsp++ = clock_gettime_nsec_np(clkid))) {
      ei->errnum = errno;
      return -1;
    }
  }
  return 0;
}

static int
collect_samples(clock_idx_t clkidx, errinfo_t *ei)
{
  int ret = -1;

  ei->errnum = errno = 0;

#define CLOCK_TYPE(name,buf) \
    case clock_type_##name: \
      ret = collect_##name(clock_args[clkidx], ei); break;
  switch (clock_types[clkidx]) {
    CLOCK_TYPES
  }
#undef CLOCK_TYPE

  return ret;
}

/* Resolution getters */

static int
getres_timeofday(void *arg, ns_time_t *np)
{
  (void) arg;
  *np = 1000;
  return 0;
}

static int
getres_mach(void *arg, ns_time_t *np)
{
  (void) arg;
  *np = mach_res;
  return 0;
}

static int
getres_gettime(void *arg, ns_time_t *np)
{
  int ret;
  timespec_t ts = {0, 0};

  ret = clock_getres((clockid_t) (pointer_int_t) arg, &ts);
  *np = ts2nsec(&ts);
  /* Resolution must be 1-1000 ns */
  if (!ret && (ts.tv_sec || ts.tv_nsec < 1 || ts.tv_nsec > 1000)) {
    ret = -err_resolution;
  }
  return ret;
}

static int
getres_gettime_ns(void *arg, ns_time_t *np)
{
  return getres_gettime(arg, np);
}

static int
getres(clock_idx_t clkidx, ns_time_t *np)
{
#define CLOCK_TYPE(name,buf) \
    case clock_type_##name: \
      return getres_##name(clock_args[clkidx], np);
  switch (clock_types[clkidx]) {
    CLOCK_TYPES
  }
#undef CLOCK_TYPE
  abort();  /* Should be unreachable, but some dumb compilers complain */
}

/* General sample collection, with checking and retries */

/* Check collected samples (nanosecond version) */
static int
check_samples(clock_idx_t clkidx, const ns_time_t buf[],
              int mode, errinfo_t *ei)
{
  int ret, maxsame, cursame = 0;
  ns_time_t last, cur = 0, res = 0, nzmin = ~0ULL;
  const ns_time_t *nsp = buf;
  sns_time_t diff;
  int numitems = mode ? NUM_DIFFS : NUM_SAMPLES;

  /* Determine maximum consecutive repeated values */
  if ((ret = getres(clkidx, &res))) return ret;
  maxsame = MAX(res, MIN_CLOCK_RES_NS) * 1000 / MIN_READ_TIME_PS;

  ei->errnum = 0;

  last = *nsp++;
  while (nsp < &buf[numitems]) {
    cur = *nsp++;
    diff = cur - last;
    if (diff < 0) {
      if (clock_okstep[clkidx]) {
        ei->errnum = -err_backstep; ret = 1; break;
      } else {
        ei->errnum = -err_illbackstep; ret = -1; break;
      }
    }
    if (diff > MAX_STEP_NS) { ei->errnum = -err_step; ret = 1; break; }
    if (!diff) {
      if (++cursame > maxsame && !clock_approx[clkidx]) {
        ei->errnum = -err_zerostep; ret = -1; break;
      }
    } else {
      if (diff < nzmin) nzmin = diff;
      cursame = 0;
    }
    last = cur;
  }

  ei->first = buf[0]; ei->last = buf[numitems - 1];
  ei->index = ret ? nsp - buf : -1;
  ei->prev = last; ei->cur = cur;
  ei->nzmin = nzmin < ~0ULL ? nzmin : 0;
  ei->badsame = cursame; ei->maxsame = maxsame;
  ei->retries = -1;  /* Will be updated */

  return ret;
}

/* Get stats on time differences */
static void
get_stats(clock_idx_t clkidx, cstats_t *sp)
{
  const ns_time_t *buf = nsbufp[clock_types[clkidx]];
  const ns_time_t *nsp = buf;
  ns_time_t last, cur;
  int64_t diff;
  long double mean, sqsum = 0.0;
  int cursame = 0;

  sp->min = ~0ULL; sp->max = 0; sp->maxsame = 0;
  last = *nsp++;
  sp->mean = mean = (buf[NUM_DIFFS] - buf[0]) / NUM_DIFFS;
  while (nsp < &buf[NUM_SAMPLES]) {
    cur = *nsp++; diff = (int64_t) cur - (int64_t) last;
    last = cur;
    if (diff < sp->min) sp->min = diff;
    if (diff > sp->max) sp->max = diff;
    sqsum = (diff - mean) * (diff - mean);
    if (!diff) {
      if (++cursame > sp->maxsame) sp->maxsame = cursame;
    } else {
      cursame =0;
    }
  }
  sp->stddev = sqrt(sqsum / (NUM_DIFFS - 1));
}

/* Get and check one clock */
static int
check_clock(clock_idx_t clkidx, cstats_t *sp, errinfo_t *ei)
{
  int ret, tries = 0;
  useconds_t sleepus = STD_SLEEP_US;

  do {
    usleepx(sleepus);
    if ((ret = collect_samples(clkidx, ei))) break;
    ret = check_samples(clkidx, nsbufp[clock_types[clkidx]], 0, ei);
    if (ret <= 0) break;
    sleepus = MIN(sleepus * 2, MAX_SLEEP_US);
  } while (++tries < MAX_STEP_TRIES);

  ei->retries = tries;
  if (!ret && sp) get_stats(clkidx, sp);
  return ret;
}

/* Report error getting time */
static void
report_clock_err(clock_idx_t clkidx, const errinfo_t *ei,
                 int mode, int verbose)
{
  const char *indent = verbose ? "    " : "";
  const char *modestr = mode < 0 ? "" : mode ? "test " : "ref ";

  printf("%s*** %sclock %s failed (%d): %s\n",
         indent, modestr, clock_names[clkidx],
         ei->errnum, get_errstr(ei->errnum));
  if (!verbose) {
    printf("  @%d: %llu->%llu(%llu), numsame = %d/%d, retries = %d\n",
           ei->index, ULL ei->prev, ULL ei->cur, ULL (ei->cur - ei->prev),
           ei->badsame, ei->maxsame, ei->retries);
  } else {
    printf("      first/last = %llu/%llu, fail @%d: %llu->%llu(%llu)\n"
           "      numsame = %d/%d, retries = %d\n",
           ULL ei->first, ULL ei->last, ei->index,
           ULL ei->prev, ULL ei->cur, ULL (ei->cur - ei->prev),
           ei->badsame, ei->maxsame, ei->retries);
  }
}

/* Print stats */
static void
print_stats(const cstats_t *sp, const errinfo_t *ei)
{
  if (sp->maxsame) {
    printf("    clock diffs (maxsame min/nzmin/mean/max stddev) "
           "= %d %llu/%llu/%.2f/%llu %.3f\n",
           sp->maxsame,
           ULL sp->min, ULL ei->nzmin, sp->mean, ULL sp->max, sp->stddev);
  } else {
    printf("    clock diffs (min/mean/max stddev) "
           "= %llu/%.2f/%llu %.3f\n",
           ULL sp->min, sp->mean, ULL sp->max, sp->stddev);
  }
  if (ei->retries > 0) {
    printf("      retries to avoid steps or excessive repeats = %d\n",
           ei->retries);
  }
}

/* Open logfile for writing */
static FILE *
open_log(clock_idx_t clkidx, const char *extra, int quiet)
{
  FILE *fp;
  const char *name = clock_names[clkidx];
  char fname[PATH_MAX];

  snprintf(fname, sizeof(fname), TEST_TEMP "/%s-%s%s.log",
           progname, name, extra);
  if(!(fp = fopen(fname, "w"))) {
    fprintf(stderr, "    Unable to open %s\n", fname);
    return NULL;
  }
  if (!quiet) printf("      Logging to " TEST_TEMP "/\n");
  return fp;
}

/* Dump all clock samples */
static void
clock_dump_ns(clock_idx_t clkidx, errinfo_t *ei, int verbose, int quiet)
{
  int idx;
  ns_time_t cur, *nsbuf = nsbufp[clock_types[clkidx]];
  sns_time_t last = nsbuf[0];
  FILE *fp;

  if (!(fp = open_log(clkidx, "", quiet))) return;
  if (verbose >= 2) {
    fprintf(fp, "# Capture of %d %s samples (%s), retries = %d\n",
            NUM_SAMPLES, clock_names[clkidx],
            ei->errnum ? "error" : "no error", ei->retries);
    fprintf(fp, "# Nonzero min delta = %d, max consecutive same = %d\n",
            (int) ei->nzmin, ei->badsame);
    fprintf(fp, "# First value = %llu, last = %llu, diff = %llu\n",
            ULL ei->first, ULL ei->last, ULL (ei->last - ei->first));
    fprintf(fp, "# Average time per sample = %.1f ns\n",
            (double) (ei->last - ei->first) / (NUM_SAMPLES - 1));
    if (ei->errnum) {
      fprintf(fp, "#\n");
      fprintf(fp, "#   *** Error %d (%s)\n",
              ei->errnum, get_errstr(ei->errnum));
      fprintf(fp, "#   *** Failed @%d: %llu -> %llu (%lld)\n",
              ei->index, ULL ei->prev, ULL ei->cur, LL ei->cur - LL ei->prev);
    }
    fprintf(fp, "#\n");
  }
  if (verbose) fprintf(fp, "# Index         Value       Delta\n");
  for (idx = 0; idx < NUM_SAMPLES; ++idx) {
    cur = nsbuf[idx];
    fprintf(fp, "%7d %19llu %5d\n", idx, ULL cur, (int) (cur - last));
    last = cur;
  }
  fclose(fp);
}

/* Report info about one clock */
static int
report_clock(clock_idx_t clkidx, int dump, int verbose, int quiet)
{
  int err, ret = 0, vnq = verbose && !quiet;
  const char *name = clock_names[clkidx];
  cstats_t stats = {0};
  errinfo_t info = {0};

  if (vnq) printf("  Checking %s", name);

  if ((err = getres(clkidx, &clock_res[clkidx]))) {
    if (!vnq) {
      printf("*** Error getting resolution of clock %s (%d): %s\n",
             clock_names[clkidx], err, get_errstr(err));
    } else {
      printf("\n    *** Error getting resolution of clock %s (%d): %s\n",
             clock_names[clkidx], err, get_errstr(err));
    }
    if (err < 0) {
      printf("      *** Value = %lld\n", (long long) clock_res[clkidx]);
    }
    return 1;
  }
  if (vnq) printf(" (resolution = %d ns)\n", (int) clock_res[clkidx]);

  if ((err = check_clock(clkidx, &stats, &info))) {
    report_clock_err(clkidx, &info, -1, verbose);
    ret = 1;
  }
  if (vnq && !ret) print_stats(&stats, &info);
  if ((dump && ret) || dump > 1) clock_dump_ns(clkidx, &info, verbose, quiet);
  return ret;
}

/* Report info about all clocks (singly) */
static int
report_all_clocks(int dump, int verbose, int quiet)
{
  int ret = 0;
  clock_idx_t clkidx = 0;

  while (clkidx < (clock_idx_t) clock_idx_max) {
    ret |= report_clock(clkidx, dump, verbose, quiet);
    ++clkidx;
  }
  return ret;
}

/* Cross-checking other clocks against mach clock */

/*
 * Functions to collect samples of target clock, interleaved with samples
 * of the basic mach clock.
 */

static int
sandwich_mach(void *arg, errinfo_t *ei)
{
  mach_time_fn_t *func = (mach_time_fn_t *) arg;
  mach_time_t *mtp = &mtbuf[0];
  ns_time_t *nsp = &mtnsbuf[0];
  mach_time_t *mtrp = &refbuf[0];
  ns_time_t *nsrp = &refnsbuf[0];

  (void) ei;

  /* Throwaways as cache warmup */
  time_scratch.mach = mach_absolute_time();
  time_scratch.mach = (*func)();

  *mtrp++ = mach_absolute_time();
  while (mtp < &mtbuf[NUM_DIFFS]) {
    *mtp++ = (*func)();
    *mtrp++ = mach_absolute_time();
  }

  mtp = &mtbuf[0];
  mtrp = &refbuf[0];

  *nsrp++ = mt2nsec(*mtrp++);
  while (nsp < &mtnsbuf[NUM_DIFFS]) {
    *nsp++ = mt2nsec(*mtp++);
    *nsrp++ = mt2nsec(*mtrp++);
  }
  return 0;
}

static int
sandwich_timeofday(void *arg, errinfo_t *ei)
{
  int ret;
  timeval_t *tvp = &tvbuf[0];
  ns_time_t *nsp = &tvnsbuf[0];
  mach_time_t *mtrp = &refbuf[0];
  ns_time_t *nsrp = &refnsbuf[0];

  (void) arg;

  /* Throwaways as cache warmup */
  time_scratch.mach = mach_absolute_time();
  (void) gettimeofday(tvp, NULL);

  *mtrp++ = mach_absolute_time();
  while (tvp < &tvbuf[NUM_DIFFS]) {
    if ((ret = gettimeofday(tvp++, NULL))) {
      ei->errnum = errno; return ret;
    }
    *mtrp++ = mach_absolute_time();
  }

  tvp = &tvbuf[0];
  mtrp = &refbuf[0];

  *nsrp++ = mt2nsec(*mtrp++);
  while (nsp < &tvnsbuf[NUM_DIFFS]) {
    if ((unsigned int) tvp->tv_usec >= MILLION) {
      ei->errnum = -err_badmicros;
      ei->badtv = *tvp;
      ret = -1;
    }
    *nsp++ = tv2nsec(tvp++);
    *nsrp++ = mt2nsec(*mtrp++);
  }
  return 0;
}

static int
sandwich_gettime(void *arg, errinfo_t *ei)
{
  int ret;
  clockid_t clkid = (clockid_t) (pointer_int_t) arg;
  timespec_t *tsp = &tsbuf[0];
  ns_time_t *nsp = &tsnsbuf[0];
  mach_time_t *mtrp = &refbuf[0];
  ns_time_t *nsrp = &refnsbuf[0];

  /* Throwaways as cache warmup */
  time_scratch.mach = mach_absolute_time();
  (void) clock_gettime(clkid, tsp);

  *mtrp++ = mach_absolute_time();
  while (tsp < &tsbuf[NUM_DIFFS]) {
    if ((ret = clock_gettime(clkid, tsp++))) {
      ei->errnum = errno; return ret;
    }
    *mtrp++ = mach_absolute_time();
  }

  tsp = &tsbuf[0];
  mtrp = &refbuf[0];

  *nsrp++ = mt2nsec(*mtrp++);
  while (nsp < &tsnsbuf[NUM_DIFFS]) {
    if ((unsigned int) tsp->tv_nsec >= BILLION) {
      ei->errnum = -err_badnanos;
      ei->badts = *tsp;
      ret = -1;
    }
    *nsp++ = ts2nsec(tsp++);
    *nsrp++ = mt2nsec(*mtrp++);
  }
  return 0;
}

static int
sandwich_gettime_ns(void *arg, errinfo_t *ei)
{
  clockid_t clkid = (clockid_t) (pointer_int_t) arg;
  ns_time_t *nsp = &tsnsbuf[0];
  mach_time_t *mtrp = &refbuf[0];
  ns_time_t *nsrp = &refnsbuf[0];

  /* Throwaways as cache warmup */
  time_scratch.mach = mach_absolute_time();
  time_scratch.ns_time = clock_gettime_nsec_np(clkid);

  *mtrp++ = mach_absolute_time();
  while (nsp < &tsnsbuf[NUM_DIFFS]) {
    if (!(*nsp++ = clock_gettime_nsec_np(clkid))) {
      ei->errnum = errno; return -1;
    }
    *mtrp++ = mach_absolute_time();
  }

  mtrp = &refbuf[0];

  while (nsrp < &refnsbuf[NUM_SAMPLES]) {
    *nsrp++ = mt2nsec(*mtrp++);
  }
  return 0;
}

static int
sandwich_samples(clock_idx_t clkidx, errinfo_t *ei)
{
  int ret = -1;

  ei->errnum = errno = 0;

#define CLOCK_TYPE(name,buf) \
    case clock_type_##name: \
      ret = sandwich_##name(clock_args[clkidx], ei); break;
  switch (clock_types[clkidx]) {
    CLOCK_TYPES
  }
#undef CLOCK_TYPE

  return ret;
}

/* Get and check one clock and reference */
static int
check_clock_sandwich(clock_idx_t clkidx, errinfo_t *ei, errinfo_t *eir)
{
  int ret, tries = 0;
  useconds_t sleepus = STD_SLEEP_US;

  do {
    ei->errnum = eir->errnum = errno = 0;
    usleepx(sleepus);
    if ((ret = sandwich_samples(clkidx, ei))) break;
    sleepus = MIN(sleepus * 2, MAX_SLEEP_US);

    ret = check_samples(clock_idx_mach_absolute, refnsbuf, 0, eir);
    if (ret) { if (ret < 0) break; continue; }

    ret = check_samples(clkidx, nsbufp[clock_types[clkidx]], 1, ei);
    if (ret <= 0) break;
  } while (++tries < MAX_STEP_TRIES);

  ei->retries = eir->retries = tries;
  return ret;
}

/* Compare clocks */
static int
compare_clocks(clock_idx_t clkidx, dstats_t *dp)
{
  const ns_time_t *refbp = &refnsbuf[0];
  const ns_time_t *testbp = nsbufp[clock_types[clkidx]];
  const ns_time_t *refp = refbp, *testp = testbp;
  sns_time_t last, cur, mean, tstcur, expected;
  sns_time_t refofs, diff;
  int numdiffs = 0;
  long double slope, diffsqr, difftot;

  /* Use mean values of straddling reference pairs as comparison reference */
  dp->refdiff = (refbp[NUM_SAMPLES - 1] + refbp[NUM_SAMPLES - 2]
                 - refbp[0] - refbp[1]) / 2;
  dp->refmean = (refbp[NUM_SAMPLES - 1] + refbp[NUM_SAMPLES - 2]
                 + refbp[0] + refbp[1]) / 4;
  dp->testdiff = testbp[NUM_DIFFS - 1] - testbp[0];
  dp->testmean = (testbp[NUM_DIFFS - 1] + testbp[0]) / 2;
  dp->offset = dp->testmean - dp->refmean;
  dp->slope = slope = (long double) dp->testdiff / dp->refdiff;

  dp->sqmin = HUGE_VAL; dp->sqmax = 0.0; difftot = 0.0;

  last = *refp++;
  while (testp < &testbp[NUM_DIFFS]) {
    tstcur = *testp++; cur = *refp++; mean = (last + cur) / 2;
    last = cur;
    refofs = mean - (sns_time_t) dp->refmean;
    expected = (sns_time_t) (refofs * slope) + dp->testmean;
    diff = tstcur - expected; diffsqr = (long double) diff * diff;
    if (diffsqr < dp->sqmin) dp->sqmin = diffsqr;
    if (diffsqr > dp->sqmax) dp->sqmax = diffsqr;
    difftot += diffsqr;
    ++numdiffs;
  }
  dp->sqmean = difftot / numdiffs;

  return 0;
}

/* Print comparison stats */
static void
print_dstats(clock_idx_t clkidx, dstats_t *dp)
{
  printf("    mach spread/mean = %llu/%llu\n",
         ULL dp->refdiff, ULL dp->refmean);
  printf("    test spread/mean = %llu/%llu\n",
         ULL dp->testdiff, ULL dp->testmean);
  if (!clock_approx[clkidx]) {
    printf("    offset = %lld, slope = %f, err = %f ppm\n",
           LL dp->offset, dp->slope, (dp->slope - 1.0) * 1E6);
  } else {
    printf("    offset = %lld, slope = %f\n",
           LL dp->offset, dp->slope);
  }
  printf("    linear fit minerr/rmserr/maxerr = %.3f/%.3f/%.3f\n",
         sqrt(dp->sqmin), sqrt(dp->sqmean), sqrt(dp->sqmax));
}

/* Dump all clock comparisons */
static void
clock_dump_dual_ns(clock_idx_t clkidx, errinfo_t *ei, errinfo_t *eir,
                   int verbose, int quiet)
{
  int idx;
  mach_time_t *rmp = refbuf;
  ns_time_t cur, next, tstcur, *nsbuf = nsbufp[clock_types[clkidx]];
  sns_time_t last = refnsbuf[0], mean, diff;
  sns_time_t ref_mean, tst_mean, mean_diff;
  FILE *fp;

  ref_mean = mt2nsec(rmp[0] + rmp[1] + rmp[NUM_DIFFS-1] + rmp[NUM_DIFFS]) / 4;
  tst_mean = (nsbuf[0] + nsbuf[NUM_DIFFS-1]) / 2;
  mean_diff = tst_mean - ref_mean;

  if (!(fp = open_log(clkidx, "-vs-mach", quiet))) return;
  if (verbose >= 2) {
    fprintf(fp, "# Comparison of %d %s samples vs. mach (%s)\n",
            NUM_DIFFS, clock_names[clkidx],
            ei->errnum ? "error" : "no error");
    fprintf(fp, "# Nonzero min ref delta = %d, min test delta = %d,"
            " max consecutive same = %d\n",
            (int) eir->nzmin, (int) ei->nzmin, ei->badsame);
    fprintf(fp, "# First ref value = %llu, last = %llu, diff = %llu\n",
            ULL eir->first, ULL eir->last, ULL (eir->last - eir->first));
    fprintf(fp, "# First test value = %llu, last = %llu, diff = %llu\n",
            ULL ei->first, ULL ei->last, ULL (ei->last - ei->first));
    fprintf(fp, "# Average time per sample = %.1f ns, retries = %d\n",
            (double) (ei->last - ei->first) / (NUM_DIFFS - 1), ei->retries);
    fprintf(fp, "# Mean ref = %lld, mean test = %lld, diff = %lld\n",
            LL ref_mean, LL tst_mean, LL mean_diff);
    if (eir->errnum) {
      fprintf(fp, "#\n");
      fprintf(fp, "#   *** Reference Clock Error %d (%s)\n",
              eir->errnum, get_errstr(eir->errnum));
      fprintf(fp, "#   *** Failed @%d: %llu -> %llu (%lld)\n",
              eir->index, ULL eir->prev, ULL eir->cur,
              LL eir->cur - LL eir->prev);
    }
    if (ei->errnum) {
      fprintf(fp, "#\n");
      fprintf(fp, "#   *** Test Clock Error %d (%s)\n",
              ei->errnum, get_errstr(ei->errnum));
      fprintf(fp, "#   *** Failed @%d: %llu\n",
              ei->index, ULL ei->cur);
    }
    fprintf(fp, "#\n");
  }
  if (verbose) fprintf(fp, "# Index         Reference   Delta"
                       "        Test_Value         Adj_Ref_Mean     Ofs\n");
  for (idx = 0; idx < NUM_DIFFS; ++idx) {
    cur = refnsbuf[idx];
    next = refnsbuf[idx+1];
    mean = (cur + next) / 2 + mean_diff;
    tstcur = nsbuf[idx];
    diff = tstcur - mean;
    fprintf(fp, "%7d %19llu %5d %19llu %+20lld %+5lld\n",
            idx, ULL cur, (int) (cur - last),
            ULL tstcur, LL mean, LL diff);
    last = cur;
  }
  cur = refnsbuf[idx];
  fprintf(fp, "%7d %19llu %5d\n",
         idx, ULL cur, (int) (cur - last));
  fclose(fp);
}

/* Report info about one clock comparison */
static int
report_clock_compare(clock_idx_t clkidx, int dump, int verbose, int quiet)
{
  int ret = 0, vnq = verbose && !quiet;
  const char *name = clock_names[clkidx];
  errinfo_t info = {0}, refinfo = {0};
  dstats_t dstats;

  if (vnq) printf("  Comparing %s to mach_absolute_time\n", name);

  if (check_clock_sandwich(clkidx, &info, &refinfo)) {
    if (refinfo.errnum) {
      report_clock_err(clock_idx_mach_absolute, &refinfo, 0, verbose);
    }
    if (info.errnum) {
      report_clock_err(clkidx, &info, 1, verbose);
    }
    ret = 1;
  } else {
    ret |= compare_clocks(clkidx, &dstats);
    if (vnq & !ret) print_dstats(clkidx, &dstats);
  }
  if ((dump && ret) || dump > 1) {
    clock_dump_dual_ns(clkidx, &info, &refinfo, verbose, quiet);
  }
  return ret;
}

/* Report info about all clock comparisons */
static int
report_all_clock_compares(int dump, int verbose, int quiet)
{
  int ret = 0;
  clock_idx_t clkidx = 0;

  while (clkidx < (clock_idx_t) clock_idx_max) {
    /* As sanity check, don't exclude self-compare */
    ret |= report_clock_compare(clkidx, dump, verbose, quiet);
    ++clkidx;
  }
  return ret;
}

/* Main function */
int
main(int argc, char *argv[])
{
  int argn = 1;
  int continuous = 0, dump = 0, quiet = 0, verbose = 0;
  int err = 0;
  const char *cp;
  char chr;

  strncpy(progname, basename(argv[0]), sizeof(progname));
  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 'C': ++continuous; break;
        case 'd': ++dump; break;
        case 'q': ++quiet; break;
        case 'v': ++verbose; break;
      }
    }
    ++argn;
  }

  if (verbose && !quiet) printf("%s starting.\n", progname);
  err = setup(verbose && !quiet);

  err |= check_invalid();

  while (!err) {
    err |= report_all_clocks(dump, verbose, quiet);
    err |= report_all_clock_compares(dump, verbose, quiet);
    if (!continuous) break;
  }

  if (!quiet) printf("%s %s.\n", progname, err ? "failed" : "passed");
  return err;
}
