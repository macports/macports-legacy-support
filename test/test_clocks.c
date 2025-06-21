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
 *
 * Since clock values are inherently nonreproducible, the challenge in clock
 * tests is to have criteria which are tight enough to detect genuine problems,
 * while being loose enough to tolerate normal variations.  In many cases, we
 * rely on retries for the latter aspect.
 *
 * To facilitate debugging, we provide both a means to capture clock samples
 * into logfiles (either conditionally on errors or unconditionally), and a
 * means to replay previously captured logs into the tests.  Note that certain
 * parameters, such as the mach_time scale and the clock resolutions, are not
 * captured in the logfiles, so it's assumed that values from the current
 * system are acceptable.  Since the replayed clocks are always the nanosecond
 * versions, matching the mach_time scale is unimportant.
 */

#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mach/mach_time.h>

#include <sys/param.h>
#include <sys/sysctl.h>
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
typedef int64_t smach_time_t;
typedef struct timeval timeval_t;
typedef struct timespec timespec_t;
typedef uint64_t ns_time_t;
typedef int64_t sns_time_t;

#define MILLION    1000000U
#define BILLION    1000000000U
#define BILLION64  1000000000ULL

/* Parameters for collection sequence */
#define MAX_STEP_NS          1000000  /* Maximum delta not considered a step */
#define MAX_APPROX_STEP_NS  15000000  /* Maximum approximate-clock step */
#define MAX_RETRIES               50  /* Maximum soft-error retries */
#define STD_SLEEP_US            1000  /* Standard sleep before collecting */
#define MAX_SLEEP_US          100000  /* Maximum sleep when retrying */
#define MAX_OFFSET_FUDGE    50000000  /* Maximum-offset fudge factor */

/* Mach_time scaling tolerance */
#define MACH_SCALE_TOLER 10E-6  /* 10 ppm */

/* Parameters (microseconds) for process/thread time test */
#define THREAD_RUN_TIME     100000  /* Wall clock time for thread runs */
#define PROCESS_SLEEP_TIME  250000  /* Overall sleep delay during test */
#define THREAD_SLEEP_MAX_CPU 10000  /* Max CPU time for sleeping thread */
#define TIME_SUM_TOLERANCE   20E-2  /* Tolerance on CPU time ratio */
#define THREAD_TEST_TRIES       10  /* Number of tries for thread time test */

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define LL (long long)
#define LLP (long long *)
#define ULL (unsigned long long)
#define ULLP (unsigned long long *)

typedef unsigned long pointer_int_t;

static mach_timebase_info_data_t tbinfo;
static long double mach2nanos, mach2usecs;
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
  ONE_ERROR(early,"Test clock too early") \
  ONE_ERROR(late,"Test clock too late") \
  ONE_ERROR(badptime,"Process/thread time inconsistent") \
  ONE_ERROR(badmach,"Bad mach_time value") \
  ONE_ERROR(badoffset,"Bad continuous time offset") \
  ONE_ERROR(badboottimelen,"Bad boottime result length") \
  ONE_ERROR(badboottimeusecs,"Bad boottime microseconds") \

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
 * Continuous (includes sleep time)
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
/* #define NP_CLOCK(name,type,okstep,okadj,approx,cont) */
#define NP_CLOCKS \
  NP_TOD_CLOCKS \
  NP_MACH_CLOCKS \
  NP_GETTIME_CLOCKS(gettime) \
  NP_GETTIME_CLOCKS(gettime_ns) \

/* Time of day clock */
#define NP_TOD_CLOCKS \
  NP_CLOCK(timeofday,timeofday,1,1,0,0) \

/* Mach clocks */
#define NP_MACH_CLOCKS \
  NP_CLOCK(absolute,mach,0,0,0,0) \
  NP_CLOCK(approximate,mach,0,0,1,0) \
  NP_CLOCK(continuous,mach,0,0,0,1) \
  NP_CLOCK(continuous_approximate,mach,0,0,1,1)

/* Gettime clocks (for both flavors) */
#define NP_GETTIME_CLOCKS(type) \
  NP_CLOCK(REALTIME,type,1,1,0,1) \
  NP_CLOCK(MONOTONIC,type,0,1,0,1) \
  NP_CLOCK(MONOTONIC_RAW,type,0,0,0,1) \
  NP_CLOCK(MONOTONIC_RAW_APPROX,type,0,0,1,1) \
  NP_CLOCK(UPTIME_RAW,type,0,0,0,0) \
  NP_CLOCK(UPTIME_RAW_APPROX,type,0,0,1,0) \

#define CALLMAC(a,b,c) a##b(c)

/* Clock type codes */
#define CLOCK_IDX_timeofday(name) clock_idx_##name
#define CLOCK_IDX_mach(name) clock_idx_mach_##name
#define CLOCK_IDX_gettime(name) clock_idx_gettime_##name
#define CLOCK_IDX_gettime_ns(name) clock_idx_gettime_ns_##name
#define NP_CLOCK(name,type,okstep,okadj,approx,cont) \
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
#define NP_CLOCK(name,type,okstep,okadj,approx,cont) \
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

#define NP_CLOCK(name,type,okstep,okadj,approx,cont) clock_type_##type,
static const clock_type_t clock_types[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx,cont) okstep,
static const int clock_okstep[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx,cont) okadj,
static const int clock_okadj[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx,cont) approx,
static const int clock_approx[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

#define NP_CLOCK(name,type,okstep,okadj,approx,cont) cont,
static const int clock_cont[] = {
  NP_CLOCKS
};
#undef NP_CLOCK

/* Arguments for collector functions */
#define CLOCK_ARG_timeofday(name) NULL
#define CLOCK_ARG_mach(name) mach_##name##_time
#define CLOCK_ARG_gettime(name) ((void *) CLOCK_##name)
#define CLOCK_ARG_gettime_ns(name) ((void *) CLOCK_##name)
#define NP_CLOCK(name,type,okstep,okadj,approx,cont) \
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
#define NP_CLOCK(name,type,okstep,okadj,approx,cont) \
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

/* Struct for info on (possible) clock errors */
typedef struct errinfo_s {
  ns_time_t first;
  ns_time_t last;
  sns_time_t difflim;
  int index;
  ns_time_t prev;
  ns_time_t cur;
  ns_time_t nzmin;
  sns_time_t lastmin;
  sns_time_t lastmax;
  sns_time_t curmin;
  sns_time_t curmax;
  int badsame;
  int maxsame;
  int retries;
  int errnum;
  timeval_t badtv;
  timespec_t badts;
} errinfo_t;

/* Struct for sleep offset info */
typedef struct sleepofs_s {
  smach_time_t ofs;
  smach_time_t toler;
  int error;
} sleepofs_t;

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

static int iteration = 0, stopiter = 0;
static char progname[PATH_MAX];

static void
intsig(int signum)
{
  (void) signum;
  ++stopiter;
}

/* Basic mach_time setup */
static int
setup(int verbose)
{
  if (mach_timebase_info(&tbinfo)) {
    printf("Can't get mach_time scale\n");
    return 1;
  }
  mach2nanos = (long double) tbinfo.numer / tbinfo.denom;
  mach2usecs = mach2nanos / 1000.0;
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

/* Get boottime via sysctl() */
static int
get_boottime(struct timeval *bt)
{
  int ret;
  size_t bt_len = sizeof(*bt);
  uint32_t *ip = (uint32_t *) bt;

  int bt_mib[] = {CTL_KERN, KERN_BOOTTIME};
  size_t bt_miblen = sizeof(bt_mib) / sizeof(bt_mib[0]);

  /* Prefill result with garbage, to detect incomplete stores */
  while (ip < (uint32_t *)(bt + 1)) {
    *ip++ = 0xDEADBEEFU;
  }

  ret = sysctl(bt_mib, bt_miblen, bt, &bt_len, NULL, 0);
  if (ret) return ret;

  /* Make sure returned length is correct */
  if (bt_len != sizeof(*bt)) {
    errno = -err_badboottimelen;
    return -1;
  }
  /* Check for valid microseconds (possibly unwritten) */
  if ((unsigned int) bt->tv_usec >= MILLION) {
    errno = -err_badboottimeusecs;
    return -1;
  }

  return 0;
}

/* Report and check boottime */
static int
check_boottime(int verbose)
{
  struct timeval bt, tod;

  if (get_boottime(&bt)) {
    printf("Can't get boottime: %s\n", get_errstr(errno));
    return 1;
  }
  if (verbose) {
    printf("  Boot time is %lld.%d\n", LL bt.tv_sec, bt.tv_usec);
  }
  if (gettimeofday(&tod, NULL)) {
    printf("Can't get timeofday: %s\n", get_errstr(errno));
    return 1;
  }
  if (tv2nsec(&bt) > tv2nsec(&tod)) {
    printf("%s*** Boot time %lld.%d is later than timeofday %lld.%d\n",
           verbose ? "    " : "",
           LL bt.tv_sec, bt.tv_usec, LL tod.tv_sec, tod.tv_usec);
    return 1;
  }

  return 0;
}

/* Version of usleep() that defends against signals */
static void
usleepx(useconds_t usecs)
{
  int ret;
  timespec_t ts;

  ts.tv_sec = usecs / MILLION;
  ts.tv_nsec = (usecs - ts.tv_sec * MILLION) * 1000;

  do {
    ret = nanosleep(&ts, &ts);
  } while (ret && errno == EINTR);
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
  int ret = 0;
  mach_time_t *mtp = &mtbuf[0];
  ns_time_t *nsp = &mtnsbuf[0];

  (void) ei;

  time_scratch.mach = (*func)();  /* Warm up cache */

  while (mtp < &mtbuf[NUM_SAMPLES]) {
    *mtp++ = (*func)();
  }

  mtp = &mtbuf[0];
  while (nsp < &mtnsbuf[NUM_SAMPLES]) {
    if ((smach_time_t) *mtp < 0) {
      ei->errnum = -err_badmach;
      ret = -1;
    }
    *nsp++ = mt2nsec(*mtp++);
  }
  return ret;
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
  sns_time_t diff, maxstep;
  int numitems = mode ? NUM_DIFFS : NUM_SAMPLES;

  /* Determine maximum consecutive repeated values */
  if ((ret = getres(clkidx, &res))) return ret;
  maxsame = MAX(res, MIN_CLOCK_RES_NS) * 1000 / MIN_READ_TIME_PS;

  /* Maximum allowable step */
  maxstep = clock_approx[clkidx] ? MAX_APPROX_STEP_NS : MAX_STEP_NS;

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
    if (diff > maxstep) {
      ei->errnum = -err_step; ret = 1; break;
    }
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
  ei->index = ret ? nsp - buf - 1 : -1;
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
      cursame = 0;
    }
  }
  sp->stddev = sqrt(sqsum / (NUM_DIFFS - 1));
}

/* Get and check one clock */
static int clock_replay_ns(clock_idx_t clkidx, int quiet);

static int
check_clock(clock_idx_t clkidx, cstats_t *sp, errinfo_t *ei,
            int quiet, int replay)
{
  int ret, tries = 0;
  useconds_t sleepus = STD_SLEEP_US;

  do {
    if (!replay) {
      usleepx(sleepus);
      if ((ret = collect_samples(clkidx, ei))) break;
    } else {
      ei->retries = -1;
      if ((ret = clock_replay_ns(clkidx, quiet))) return ret;
    }
    ret = check_samples(clkidx, nsbufp[clock_types[clkidx]], 0, ei);
    if (ret <= 0) break;
    sleepus = MIN(sleepus * 2, MAX_SLEEP_US);
  } while (!replay && ++tries < MAX_RETRIES);

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
    printf("      last min/max = %lld/%lld, cur min/max = %lld/%lld\n",
           LL ei->lastmin, LL ei->lastmax, LL ei->curmin, LL ei->curmax);
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
open_log(clock_idx_t clkidx, const char *extra, int quiet, int replay)
{
  FILE *fp;
  const char *name = clock_names[clkidx];
  char fname[PATH_MAX];

  if (!iteration) {
    snprintf(fname, sizeof(fname), TEST_TEMP "/%s-%s%s.log",
             progname, name, extra);
  } else {
    snprintf(fname, sizeof(fname), TEST_TEMP "/%s-%s%s-%d.log",
             progname, name, extra, iteration);
  }
  if(!(fp = fopen(fname, replay ? "r" : "w"))) {
    if (replay && errno == ENOENT) {
      if (!quiet) fprintf(stderr, "    Skipping nonexistent %s\n", fname);
      return NULL;
    }
    fprintf(stderr, "    Unable to open %s\n", fname);
    return NULL;
  }
  if (!quiet) printf("      %s %s\n",
                     replay ? "Replaying from" : "Logging to", fname);
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

  if (!(fp = open_log(clkidx, "", quiet, 0))) return;
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
    fprintf(fp, " %6d %19llu %5d\n", idx, ULL cur, (int) (cur - last));
    last = cur;
  }
  fclose(fp);
}

/* Replay data from prior dump */
static int
clock_replay_ns(clock_idx_t clkidx, int quiet)
{
  int ret = 0, count, idx = -1, diff;
  char pfx, eol, buf[256];
  ns_time_t cur, *nsbuf = nsbufp[clock_types[clkidx]];
  FILE *fp;

  if (!(fp = open_log(clkidx, "", quiet, 1))) return -1;
  do {
    if (!fgets(buf, sizeof(buf), fp)) {
      ret = !feof(fp);
      break;
    }
    count = sscanf(buf, "%c%d %llu %d%c",
                   &pfx, &idx, ULLP &cur, &diff, &eol);
    if (count < 0) {
      ret = 1;
      break;
    }
    if (pfx == '#') continue;
    if (pfx != ' ' || eol != '\n' || count != 5) {
      if (!quiet) fprintf(stderr, "    Bad line at/after index %d\n", idx);
      ret = -1;
      break;
    }
    if (idx < 0 || idx >= NUM_SAMPLES) {
      if (!quiet) fprintf(stderr, "    Skipping bad index %d\n", idx);
      continue;
    }
    nsbuf[idx] = cur;
  } while (1);

  fclose(fp);
  return ret;
}

/* Report info about one clock */
static int
report_clock(clock_idx_t clkidx, int dump, int dverbose,
             int verbose, int quiet, int replay)
{
  int err, ret = 0, vnq = verbose && !quiet;
  const char *name = clock_names[clkidx];
  cstats_t stats = {0};
  errinfo_t info = {0};

  if (vnq) printf("  Checking %s", name);

  /* In replay mode, assume that current resolutions are acceptable */
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

  if ((err = check_clock(clkidx, &stats, &info, quiet, replay))) {
    if (replay && info.retries < 0) return 0;  /* Just skip nonex replay */
    report_clock_err(clkidx, &info, -1, verbose);
    ret = 1;
  }
  if (vnq && !ret) print_stats(&stats, &info);
  if ((dump && ret) || dump > 1) clock_dump_ns(clkidx, &info, dverbose, quiet);
  return ret;
}

/* Report info about all clocks (singly) */
static int
report_all_clocks(int dump, int dverbose, int verbose, int quiet, int replay)
{
  int ret = 0;
  clock_idx_t clkidx = 0;

  while (clkidx < (clock_idx_t) clock_idx_max) {
    ret |= report_clock(clkidx, dump, dverbose, verbose, quiet, replay);
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
  int ret = 0;
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
    if ((smach_time_t) *mtp < 0) {
      ei->errnum = -err_badmach;
      ret = -1;
    }
    *nsp++ = mt2nsec(*mtp++);
    *nsrp++ = mt2nsec(*mtrp++);
  }
  return ret;
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

/*
 * Check test clock against reference.
 *
 * In general there is a potentially large offset between the two clocks,
 * but we check the offset behavior for consistency.  The general idea is
 * to compute minimum and maximum differences, based on the previous and
 * subsequent reference values, adjusted by a tolerance value.  The behavior
 * is considered correct when each min/max interval overlaps the preceding
 * min/max interval.
 *
 * Steerable clocks complicate this.  Not only do explicit step adjustments
 * (which are mostly but not completely filtered out earlier) violate this
 * criterion, but even "slewing" adjustments involve periodic small step
 * adjustments which break things as well.  To allow for this, we make
 * the overlap failure a retriable error on steerable clocks; otherwise
 * it's fatal.
 *
 * In principle, at most one such step should be allowed within a given
 * sample set, but we don't bother to check for that.
 *
 * A similar issue exists with approximate clocks, which stay at the same
 * value for a long time and then jump significantly.  We do a few things
 * to allow for this:
 *   1) We don't start checking values until the first step occurs.  This
 *   allows capturing the delay offset from the update.
 *   2) Additionally, we use the one-step-earlier version of the reference
 *   to compute the minimum, to allow for the earlier capture of the
 *   approximate clock.
 *   2) We keep the "lastmax" value unchnaged when the clock value is
 *   unchanged, providing extra tolerance for the step.
 * In addition to the above, the "late" case is considered retriable.
 */
static int
compare_clocks(clock_idx_t clkidx, const sleepofs_t *so,
               errinfo_t *ei, errinfo_t *eir)
{
  const ns_time_t *refbp = &refnsbuf[0];
  const ns_time_t *testbp = nsbufp[clock_types[clkidx]];
  const ns_time_t *refp = refbp, *testp = testbp;
  sns_time_t prev, last, cur, tstlast, tstcur = 0;
  int ret = 0, steps = 0, adjret = clock_okadj[clkidx] ? 1 : -1;
  sns_time_t minref = 0, maxref = 0, difflim, ofslim;
  sns_time_t lastmin = INT64_MIN, lastmax = INT64_MAX, curmin = 0, curmax = 0;

  /*
   * The maximum discrepancy between clocks (after accounting for the offset)
   * is the sum of the resolutions of the clocks.  But since the resolution
   * is reported overoptimistically on some platforms (x86), we use the larger
   * of the reported resolution and the observed minimum nonzero difference.
   */
  difflim = MAX(clock_res[clkidx], ei->nzmin)
            + MAX(clock_res[clock_idx_mach_absolute], eir->nzmin);

  /*
   * The maximum offset for continuous clocks is the maximum sleep offset
   * plus the difflim, plus a generous fudge factor.  Non-continuous clocks
   * are allowed the same offset without the sleep offset, except that
   * adjustable clocks don't get this check at all.
   *
   * If the sleep offset is unavailable, it's assumed that the continuous
   * clocks don't apply it, and hence are treated as non-continuous.
   */
  if (clock_okadj[clkidx]) {
    ofslim = INT64_MAX;
  } else if (clock_cont[clkidx]) {
    ofslim = mt2nsec(llabs(so->ofs) + llabs(so->toler))
             + difflim + MAX_OFFSET_FUDGE;
  } else {
    ofslim = difflim + MAX_OFFSET_FUDGE;
  }

  tstlast = *testp; prev = last = *refp++;
  while (testp < &testbp[NUM_DIFFS]) {
    tstcur = *testp++; cur = *refp++;
    minref = (clock_approx[clkidx] ? prev : last) - difflim;
    maxref = cur + difflim;
    curmin = tstcur - maxref; curmax = tstcur - minref;
    if (llabs(tstcur - cur) > ofslim) {
      ei->errnum = -err_badoffset;
      ret = -1;
      break;
    }
    if (!clock_approx[clkidx]) {
      if (curmax < lastmin) {
        ei->errnum = -err_early;
        ret = adjret;
        break;
      }
      if (curmin > lastmax) {
        ei->errnum = -err_late;
        ret = adjret;
        break;
      }
      lastmax = curmax;
    } else {
      if (tstcur != tstlast || steps) {
        if (curmax < lastmin) {
          ei->errnum = -err_early;
          ret = -1;
          break;
        }
        if (curmin > lastmax) {
          ei->errnum = -err_late;
          ret = 1;
          break;
        }
      }
      if (tstcur != tstlast) {
        ++steps;
        lastmax = curmax;
      }
    }
    tstlast = tstcur; prev = last; last = cur; lastmin = curmin;
  }

  ei->first = testbp[0]; ei->last = testbp[NUM_DIFFS - 1];
  ei->difflim = difflim;
  ei->index = ret ? testp - testbp - 1 : -2;
  ei->prev = ei->cur = tstcur;
  ei->lastmin = lastmin; ei->lastmax = lastmax;
  ei->curmin = curmin; ei->curmax = curmax;
  ei->retries = -1;

  return ret;
}

/* Get stats on comparison */
static void
get_dstats(clock_idx_t clkidx, dstats_t *dp)
{
  const ns_time_t *refbp = &refnsbuf[0];
  const ns_time_t *testbp = nsbufp[clock_types[clkidx]];
  const ns_time_t *refp = refbp, *testp = testbp;
  sns_time_t last, cur, mean, tstcur, expected;
  sns_time_t refofs, diff;
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
    tstcur = *testp++; cur = *refp++;
    mean = (last + cur) / 2;
    refofs = mean - (sns_time_t) dp->refmean;
    expected = (sns_time_t) (refofs * slope) + dp->testmean;
    diff = tstcur - expected; diffsqr = (long double) diff * diff;
    if (diffsqr < dp->sqmin) dp->sqmin = diffsqr;
    if (diffsqr > dp->sqmax) dp->sqmax = diffsqr;
    difftot += diffsqr;
  }

  dp->sqmean = difftot / NUM_DIFFS;
}

/* Get and check one clock and reference */
static int clock_replay_dual_ns(clock_idx_t clkidx, int quiet);

static int
check_clock_sandwich(clock_idx_t clkidx, const sleepofs_t *so,
                     errinfo_t *ei, errinfo_t *eir, int quiet, int replay)
{
  int ret, tries = 0;
  useconds_t sleepus = STD_SLEEP_US;

  do {
    ei->errnum = eir->errnum = errno = 0;
    if (!replay) {
      usleepx(sleepus);
      if ((ret = sandwich_samples(clkidx, ei))) break;
    } else {
      ei->retries = -1;
      if ((ret = clock_replay_dual_ns(clkidx, quiet))) return ret;
    }
    sleepus = MIN(sleepus * 2, MAX_SLEEP_US);

    ret = check_samples(clock_idx_mach_absolute, refnsbuf, 0, eir);
    if (ret) { if (ret < 0) break; continue; }

    ret = check_samples(clkidx, nsbufp[clock_types[clkidx]], 1, ei);
    if (ret) { if (ret < 0) break; continue; }
    ret = compare_clocks(clkidx, so, ei, eir);
    if (ret <= 0) break;
  } while (!replay && ++tries < MAX_RETRIES);

  ei->retries = eir->retries = tries;
  return ret;
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
  ns_time_t cur, next, tstcur, *nsbuf = nsbufp[clock_types[clkidx]];
  sns_time_t last = refnsbuf[0], meanofs, tstmin, tstmax, mean, diff;
  FILE *fp;

  meanofs = (sns_time_t) (ei->first + ei->last - eir->first - eir->last) / 2;

  if (!(fp = open_log(clkidx, "-vs-mach", quiet, 0))) return;
  if (verbose >= 2) {
    fprintf(fp, "# Comparison of %d %s samples vs. mach (%s)\n",
            NUM_DIFFS, clock_names[clkidx],
            ei->errnum ? "error" : "no error");
    fprintf(fp, "# Nonzero min ref delta = %d, min test delta = %d,"
            " max consecutive same = %d\n",
            (int) eir->nzmin, (int) ei->nzmin, ei->badsame);
    fprintf(fp, "# Average time per sample = %.1f ns, diff tolerance = %lld,"
            " retries = %d\n",
            (double) (ei->last - ei->first) / (NUM_DIFFS - 1),
            LL ei->difflim, ei->retries);
    fprintf(fp, "# First ref value = %llu, last = %llu, diff = %llu\n",
            ULL eir->first, ULL eir->last, ULL (eir->last - eir->first));
    fprintf(fp, "# First test value = %llu, last = %llu, diff = %llu\n",
            ULL ei->first, ULL ei->last, ULL (ei->last - ei->first));
    fprintf(fp, "# Mean diff = %lld (subtracted from Diff, Rmin, Rmax)\n",
            LL meanofs);
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
      fprintf(fp, "#   *** Last min/max = %lld/%lld\n",
              LL ei->lastmin, LL ei->lastmax);
      fprintf(fp, "#   *** Current min/max = %lld/%lld\n",
              LL ei->curmin, LL ei->curmax);
    }
    fprintf(fp, "#\n");
  }
  if (verbose) fprintf(fp, "# Index         Reference   Delta"
                       "        Test_Value     Diff"
                       "   Rmin   Rmax\n");
  for (idx = 0; idx < NUM_DIFFS; ++idx) {
    cur = refnsbuf[idx];
    next = refnsbuf[idx+1];
    tstcur = nsbuf[idx];
    mean = (cur + next) / 2;  diff = (sns_time_t) tstcur - mean - meanofs;
    tstmin = tstcur - (next + ei->difflim);
    tstmax = tstcur - (cur - ei->difflim);
    fprintf(fp, " %6d %19llu %5d %19llu %+5lld %+5lld %+5lld\n",
            idx, ULL cur, (int) (cur - last),
            ULL tstcur, LL diff, LL tstmin - meanofs, LL tstmax - meanofs);
    last = cur;
  }
  cur = refnsbuf[idx];
  fprintf(fp, " %6d %19llu %5d\n",
         idx, ULL cur, (int) (cur - last));
  fclose(fp);
}

/* Replay data from prior dump */
static int
clock_replay_dual_ns(clock_idx_t clkidx, int quiet)
{
  int ret = 0, count, idx = -1, diff1, diff2, ofs1, ofs2;
  char pfx, sep, eol, buf[256];
  ns_time_t cur, tstcur, *nsbuf = nsbufp[clock_types[clkidx]];
  FILE *fp;

  if (!(fp = open_log(clkidx, "-vs-mach", quiet, 1))) return -1;
  do {
    if (!fgets(buf, sizeof(buf), fp)) {
      ret = !feof(fp);
      break;
    }
    count = sscanf(buf, "%c%d %llu %d%c%llu %lld %lld %lld%c",
                   &pfx, &idx, ULLP &cur, &diff1, &sep,
                   ULLP &tstcur, LLP &diff2, LLP &ofs1, LLP &ofs2, &eol);
    if (count < 0) {
      ret = 1;
      break;
    }
    if (pfx == '#') continue;
    if (pfx != ' '
        || !((count == 10 && sep == ' ' && eol == '\n')
             || (count == 5 && sep == '\n'))) {
      if (!quiet) fprintf(stderr, "    Bad line at/after index %d\n", idx);
      ret = -1;
      break;
    }
    if (idx < 0 || idx >= NUM_SAMPLES
        || (idx >= NUM_DIFFS && sep == ' ')) {
      if (!quiet) fprintf(stderr, "    Skipping bad index %d\n", idx);
      continue;
    }
    if (sep == ' ') nsbuf[idx] = tstcur;
    refnsbuf[idx] = cur;
  } while (1);

  fclose(fp);
  return ret;
}

/* Report info about one clock comparison */
static int
report_clock_compare(clock_idx_t clkidx, const sleepofs_t *so,
                     int dump, int dverbose, int verbose, int quiet, int replay)
{
  int ret = 0, vnq = verbose && !quiet;
  const char *name = clock_names[clkidx];
  errinfo_t info = {0}, refinfo = {0};
  dstats_t dstats;

  if (vnq) printf("  Comparing %s to mach_absolute_time\n", name);

  if (check_clock_sandwich(clkidx, so, &info, &refinfo, quiet, replay)) {
    if (replay && info.retries < 0) return 0;  /* Just skip nonex replay */
    if (refinfo.errnum) {
      report_clock_err(clock_idx_mach_absolute, &refinfo, 0, verbose);
    }
    if (info.errnum) {
      report_clock_err(clkidx, &info, 1, verbose);
    }
    ret = 1;
  } else {
    if (vnq) {
      get_dstats(clkidx, &dstats);
      print_dstats(clkidx, &dstats);
    }
  }
  if ((dump && ret) || dump > 1) {
    clock_dump_dual_ns(clkidx, &info, &refinfo, dverbose, quiet);
  }
  return ret;
}

/* Report info about all clock comparisons */
static int
report_all_clock_compares(const sleepofs_t *so, int dump, int dverbose,
                          int verbose, int quiet, int replay)
{
  int ret = 0;
  clock_idx_t clkidx = 0;

  while (clkidx < (clock_idx_t) clock_idx_max) {
    /* As sanity check, don't exclude self-compare */
    ret |= report_clock_compare(clkidx, so,
                                dump, dverbose, verbose, quiet, replay);
    ++clkidx;
  }
  return ret;
}

/*
 * Check mach_time scaling
 *
 * This checks the computation that scales mach_time to nanoseconds.  It
 * only checks the accuracy of applying the scale factor reported by the OS;
 * it does *not* check the scale factor itself.  See the comment titled
 * "Mach timebase scaling" in src/time.c for details of right and wrong ways
 * to do this.
 *
 * The basic approach is to grab a "sandwich" of CLOCK_UPTIME_RAW (ns)
 * between samples of mach_absolute_time(), and check the ratio of values.
 * With any uptime of more than about a second, the inaccuracy of the
 * "divide first" approach on PowerPC is reliably observable, and with any
 * uptime of more than about 7.4 minutes, the overflow and wraparound of the
 * "multiply first" approach on PowerPC can be observed.
 *
 * If and when the overflow occurs from using the "multiply first" approach
 * on PowerPC, the computed nanosecond value wraps around, after which its
 * value is never more than half the correct value.  Thus, it can be detected
 * on the basis of a very large negative error in the scale (at least 0.5).
 */
static int
check_mach_scaling(int verbose)
{
  int ret = 0, idx, best = 0;
  mach_time_t mt[4];
  ns_time_t nst[3];
  double nslo, nshi, nsmean, nstoler, nscur, nserr, err_mult;
  const char *err_units, *msg;

  if (verbose) printf("  Checking mach_time scaling\n");

  /* Get tightest "sandwich" out of 4/3 */
  mt[0] = mach_absolute_time();
  nst[0] = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  mt[1] = mach_absolute_time();
  nst[1] = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  mt[2] = mach_absolute_time();
  nst[2] = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  mt[3] = mach_absolute_time();

  for(idx = 1; idx < 3; ++idx) {
    if (mt[idx+1] - mt[idx] < mt[best+1] - mt[best]) best = idx;
  }

  nslo = mt[best] * mach2nanos; nshi = mt[best+1] * mach2nanos;
  nscur = nst[best];

  nsmean = (nslo + nshi) / 2.0;
  nstoler = (nshi - nslo) / 2.0 / nsmean;
  nserr = (nscur - nsmean) / nsmean;

  if (nserr < -0.499) {
    printf("    *** Mach scaling wraparound, mach = %llu->%llu, ns = %llu\n",
           ULL mt[best], ULL mt[best+1], ULL nst[best]);
    return 1;
  }

  if (fabs(nserr) >= 1E-6) {
    err_mult = 1E6; err_units = "ppm";
  } else {
    err_mult = 1E9; err_units = "ppb";
  }
  if (fabs(nserr) >= MACH_SCALE_TOLER) ret = 1;

  if (verbose || ret) {
    msg = ret ? "*** Excessive mach scaling error"
              : "Measured mach scaling error";
    printf("    %s = %+.6f %s +/- %.6f %s\n", msg,
           nserr * err_mult, err_units, nstoler * err_mult, err_units);
  }

  return ret;
}

/*
 * Check process and thread clocks
 *
 * This launches three subthreads, two spinning and one sleeping, letting
 * them run for PROCESS_SLEEP_TIME, and then checking the reported times
 * for consistency, where "consistency" means:
 *
 *   1) Neither spinning thread time exceeds wall-clock time.
 *   2) The sleeping thread time doesn't exceed THREAD_SLEEP_MAX_CPU.
 *   3) The total thread time matches the process's within TIME_SUM_TOLERANCE.
 *
 * In addition, each time capture sandwiches a timespec version between
 * two nanosecond versions, and checks the triplet for consistency.
 */

/*
 * Rosetta 2 bug
 *
 * There is a bug in all current versions of Rosetta 2 that badly screws up
 * thread times.  Rosetta 2 pretends to have a 1GHz mach clock, to be more
 * like real x86 systems, and appropriately compensates for this in most
 * clock computations.  But the thread CPU-time calculation fails to do this,
 * resulting in values that are off by a factor of the true mach-time ratio.
 * E.g., on the M1 (mach ratio 125/3), the thread time is underreported by a
 * factor of ~41.7.
 *
 * Note that this is not a legacy-support bug, since it only applies to OS
 * versions that don't use the legacy-support implementations of the clock
 * functions.  Nevertheless, the test is expected to pass on all OS versions,
 * so we need to disable the failures in this case.  In the interests of full
 * disclosure, we don't disable the error messages; we only disable the
 * failures.
 *
 * Although it might be possible to compensate for the error, this would be
 * an unnecessary complication for a test primarily intended to test the
 * legacy-support code, not work around Apple's bugs.  It also might be
 * nontrivial to obtain the true mach-time scale factor when running under
 * Rosetta 2.
 */

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 110000 \
    && defined(__x86_64__)

#include <sys/sysctl.h>

/* sysctl to check whether we're running in Rosetta 2 */
#define SYSCTL_TRANSLATED "sysctl.proc_translated"

/* Test whether running under Rosetta */
/* 0 no, 1 yes */
static int
check_rosetta(void)
{
  int translated;
  size_t translated_sz = sizeof(translated);

  if (sysctlbyname(SYSCTL_TRANSLATED, &translated, &translated_sz,
                   NULL, 0) < 0) {
    /* If sysctl failed, must be really native. */
    return 0;
  }
  return translated ? 1 : 0;
}

#else /* Not OS 11.x+ x86_64 */

static int check_rosetta(void) { return 0;}

#endif /* Not OS 11.x+ x86_64 */

/* Now back to our regularly scheduled clock tests */

typedef void * (pthread_fn_t)(void *);

/* Struct for both nsec and timespec captures */
typedef struct ptime_s {
  volatile ns_time_t before;
  volatile ns_time_t middle;
  volatile ns_time_t after;
  volatile timespec_t ts;
  volatile int ret;
  volatile int errnum;
} ptime_t;

/* Struct with start and end times, plus other info */
typedef struct dptime_s {
  ptime_t start;
  ptime_t end;
  pthread_t id;
  volatile int ret;
  volatile int errnum;
  pthread_fn_t *func;
  const char *name;
} dptime_t;

/* Get specified time two ways, and compare */
static int
get_ptime(clockid_t clkid, ptime_t *pt)
{
  int ret = 0;
  timespec_t ts;

  pt->errnum = 0;
  do {
    errno = -err_noerrno;
    if (!(pt->before = clock_gettime_nsec_np(clkid))) {
      pt->errnum = errno;
      ret = -2; break;
    }
    errno = -err_noerrno;
    if ((ret = clock_gettime(clkid, &ts))) {
      pt->errnum = errno;
      break;
    }
    errno = -err_noerrno;
    if (!(pt->after = clock_gettime_nsec_np(clkid))) {
      pt->errnum = errno;
      ret = -3; break;
    }

    if ((unsigned int) ts.tv_nsec >= BILLION) {
      pt->errnum = -err_badnanos;
      ret = -4; break;
    }

    pt->ts = ts;
    pt->middle = ts2nsec(&ts);
    if (pt->before > pt->middle || pt->middle > pt->after) {
      pt->errnum = -err_badptime;
      ret = -5; break;
    }
  } while (0);

  pt->ret = ret;

  return ret;
}

/* Start a given thread */
static int
start_thread(dptime_t *dpt)
{
  int ret;

  dpt->ret = dpt->errnum = 0;
  if ((ret = pthread_create(&dpt->id, NULL, dpt->func, (void *)dpt))) {
    dpt->ret = ret;
    dpt->errnum = errno;
  }

  return ret;
}

/* Stop multiple threads */
static void
stop_threads(dptime_t *threads[], int numthreads)
{
  while (--numthreads >= 0) {
    (void) pthread_cancel(threads[numthreads]->id);
  }
}

/* Start multiple threads */
static dptime_t *
start_threads(dptime_t *threads[], int numthreads)
{
  int thread, ret = 0;

  for (thread = 0; thread < numthreads; ++thread) {
    ret = start_thread(threads[thread]);
    if (ret) {
      stop_threads(threads, thread);
      return threads[thread];
    }
  }
  return NULL;
}

/* Wait for threads to stop */
static int
wait_threads(dptime_t *threads[], int numthreads)
{
  int ret = 0;
  dptime_t *dpt;

  while (--numthreads >= 0) {
    dpt = threads[numthreads];
    if ((dpt->ret = pthread_join(dpt->id, NULL))) {
      dpt->errnum = errno;
      ret = 1;
    }
  }
  return ret;
}

/* Report errors from threads */
static int
report_thread_errs(dptime_t *threads[], int numthreads)
{
  int ret = 0, thread;
  dptime_t *dpt;

  for (thread = 0; thread < numthreads; ++thread) {
    dpt = threads[thread];
    if (dpt->start.ret) {
      printf("    *** Error (%d) getting thread %s start time (%d): %s\n",
             dpt->start.ret, dpt->name,
             dpt->start.errnum, get_errstr(dpt->start.errnum));
      ret = 1;
    }
    if (dpt->end.ret) {
      printf("    *** Error (%d) getting thread %s end time (%d): %s\n",
             dpt->end.ret, dpt->name,
             dpt->end.errnum, get_errstr(dpt->end.errnum));
      ret = 1;
    }
  }
  return ret;
}

/* Report time range for one process or thread */
static void
report_times(dptime_t *dpt, int verbose)
{
  const char *tp = dpt->name ? "Thread" : "Process";
  const char *tn = dpt->name ? dpt->name : "";

  if (verbose < 2) {
    printf("    %s %s times %llu -> %llu, = %.6f ms\n",
           tp, tn, ULL dpt->start.middle, ULL dpt->end.middle,
           (dpt->end.middle - dpt->start.middle) / 1E6);
  } else {
    printf("    %s %s times %llu/%llu/%llu -> %llu/%llu/%llu,"
           " = %.6f/%.6f/%.6f ms\n",
           tp, tn,
           ULL dpt->start.before, ULL dpt->start.middle, ULL dpt->start.after,
           ULL dpt->end.before, ULL dpt->end.middle, ULL dpt->end.after,
           (dpt->end.before - dpt->start.after) / 1E6,
           (dpt->end.middle - dpt->start.middle) / 1E6,
           (dpt->end.after - dpt->start.before) / 1E6);
  }
}

/* Report times from all threads */
static void
report_thread_times(dptime_t *threads[], int numthreads, int verbose)
{
  int thread;

  for (thread = 0; thread < numthreads; ++thread) {
    report_times(threads[thread], verbose);
  }
}

/* Spinning thread */
static void *
thread_spin(void *arg)
{
  dptime_t *dpt = (dptime_t *) arg;
  mach_time_t stop_time;

  if (get_ptime(CLOCK_THREAD_CPUTIME_ID, &dpt->start)) return NULL;
  stop_time = mach_absolute_time() + THREAD_RUN_TIME / mach2usecs;
  while (mach_absolute_time() < stop_time) ;
  (void) get_ptime(CLOCK_THREAD_CPUTIME_ID, &dpt->end);
  return NULL;
}

/* Sleeping thread */
static void *
thread_sleep(void *arg)
{
  dptime_t *dpt = (dptime_t *) arg;

  if (get_ptime(CLOCK_THREAD_CPUTIME_ID, &dpt->start)) return NULL;
  usleepx(THREAD_RUN_TIME);
  (void) get_ptime(CLOCK_THREAD_CPUTIME_ID, &dpt->end);
  return NULL;
}

/* Main checking function */
static int
check_thread_times(int verbose, int quiet)
{
  int ret = 0, rret = 0, wret = 0, eret = 0;
  ns_time_t start, end, wall, difflo, diffhi;
  dptime_t spin1 = {.func = thread_spin, .name = "spin1"};
  dptime_t spin2 = {.func = thread_spin, .name = "spin2"};
  dptime_t sleeper = {.func = thread_sleep, .name = "sleeper"};
  dptime_t top = {.func = NULL};
  dptime_t *threads[] = {&spin1, &spin2, &sleeper};
  const int numthreads = sizeof(threads) / sizeof(threads[0]);
  dptime_t *failed;
  double proclo, prochi, threadlo = 0.0, threadhi = 0.0;
  int rosetta = check_rosetta();
  int rquiet = rosetta && quiet;

  if (verbose && !quiet) printf("  Checking thread times.\n");

  start = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);

  if (get_ptime(CLOCK_PROCESS_CPUTIME_ID, &top.start)) {
    printf("    *** Error getting process starting time (%d): %s\n",
           top.errnum, get_errstr(top.errnum));
    return -1;
  }

  if ((failed = start_threads(threads, numthreads))) {
    printf("    *** Error starting threads (%d): %s\n",
           failed->errnum, get_errstr(failed->errnum));
    return -1;
  }

  usleepx(PROCESS_SLEEP_TIME);

  wret = wait_threads(threads, numthreads); 

  eret = get_ptime(CLOCK_PROCESS_CPUTIME_ID, &top.end);

  end = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  wall = end - start;

  if (wret) {
    stop_threads(threads, numthreads);
  }

  if (wret) {
    printf("    *** Error waiting for threads, running = %d\n", wret);
    ret = -1;
  }
  if (eret) {
    printf("    *** Error getting process ending time (%d): %s\n",
           top.errnum, get_errstr(top.errnum));
    ret = -1;
  }
  
  ret |= report_thread_errs(threads, numthreads);

  if (verbose && !quiet && !ret) {
    printf("    Total wall clock time = %.3f ms\n", wall / 1E6);
    report_thread_times(threads, numthreads, verbose);
    report_times(&top, verbose);
  }

  if (!ret) {
  
    difflo = spin1.end.before - spin1.start.after;
    diffhi = spin1.end.after - spin1.start.before;
    threadlo += difflo / 1E6; threadhi += diffhi / 1E6;
    if (diffhi > wall) {
      if (!rquiet) {
        printf("    *** Spin1 CPU %llu/%llu exceeds wall time %llu\n",
               ULL difflo, ULL diffhi, ULL wall);
      }
      if (rosetta) rret = 1; else ret = 1;
    }
  
    difflo = spin2.end.before - spin2.start.after;
    diffhi = spin2.end.after - spin2.start.before;
    threadlo += difflo / 1E6; threadhi += diffhi / 1E6;
    if (diffhi > wall) {
      if (!rquiet) {
        printf("    *** Spin2 CPU %llu/%llu exceeds wall time %llu\n",
               ULL difflo, ULL diffhi, ULL wall);
      }
      if (rosetta) rret = 1; else ret = 1;
    }
  
    difflo = sleeper.end.before - sleeper.start.after;
    diffhi = sleeper.end.after - sleeper.start.before;
    threadlo += difflo / 1E6; threadhi += diffhi / 1E6;
    if (diffhi > THREAD_SLEEP_MAX_CPU * 1000) {
      if (!rquiet) {
        printf("    *** Sleeper CPU %llu/%llu exceeds limit %llu\n",
               ULL difflo, ULL diffhi, ULL THREAD_SLEEP_MAX_CPU * 1000);
      }
      if (rosetta) rret = 1; else ret = 1;
    }

    proclo = (top.end.before - top.start.after) / 1E6;
    prochi = (top.end.after - top.start.before) / 1E6;
    if (fabs(threadlo / prochi - 1.0) > TIME_SUM_TOLERANCE
        || fabs(threadhi / proclo - 1.0) > TIME_SUM_TOLERANCE) {
      if (!rquiet) {
        printf("    *** Total thread time %.6f/%.6f ms"
               " mismatches process time %.6f/%.6f ms\n",
               threadlo, threadhi, proclo, prochi);
      }
      if (rosetta) rret = 1; else ret = 1;
    }

    if (verbose && !quiet) {
      printf("    Total thread time = process time %+.2f%%/%.2f%%\n",
             (threadlo / prochi - 1.0) * 100.0,
             (threadhi / proclo - 1.0) * 100.0);
    }

    if (rret && !ret && !quiet) {
      printf("      Ignoring errors caused by Rosetta 2 bug\n");
    }
  }

  return ret;
}

/* Sleep offset (continuous - absolute time) functions */

/* Obtain sleep offset */
static void
get_sleepofs(sleepofs_t *ofs)
{
  int idx = 0;
  mach_time_t std[4], *stp = &std[0];
  mach_time_t cont[3], *ctp = &cont[0];

  /*
   * It's been empirically determined that a 4+3 sandwich is usually good
   * enough to get a "best" sandwich.
   */
  *stp++ = mach_absolute_time();
  *ctp++ = mach_continuous_time();
  *stp++ = mach_absolute_time();
  *ctp++ = mach_continuous_time();
  *stp++ = mach_absolute_time();
  *ctp++ = mach_continuous_time();
  *stp++ = mach_absolute_time();

  /* If any continuous times are negative, sleep offset is poisoned. */
  if ((smach_time_t) (cont[0] | cont[1] | cont[2]) < 0) {
    ofs->ofs = ofs->toler = 0;
    ofs->error = 1;
    return;
  }

  /* Find tightest sandwich */
  if (std[idx+2] - std[idx+1] < std[idx+1] - std[idx]) ++idx;
  if (std[idx+2] - std[idx+1] < std[idx+1] - std[idx]) ++idx;

  ofs->ofs = cont[idx] - (std[idx+1] + std[idx]) / 2;
  /* Round up and add one unit to tolerance */
  ofs->toler = ((std[idx+1] - std[idx]) + 1 + 2) / 2;
  ofs->error = 0;
}

/* Compare sleep offsets */
static int
compare_sleepofs(const sleepofs_t *ofs1, const sleepofs_t *ofs2)
{
  /* Return 0 if ranges overlap */
  if (ofs1->ofs + ofs1->toler >= ofs2->ofs - ofs2->toler) return 0;
  if (ofs2->ofs + ofs2->toler >= ofs1->ofs - ofs1->toler) return 0;
  /* Else return comparison */
  return ofs1->ofs < ofs2->ofs ? -1 : 1;
}

/* Report sleep offset */
static void
report_sleepofs(const char *text, const sleepofs_t *ofs)
{
  long double nanos = ofs->ofs * mach2nanos;
  double scaled;
  const char *units;

  if (ofs->error) {
    printf("%s is unavailable\n", text);
    return;
  }
  if (nanos < 1E9) {
    scaled = nanos / 1E6; units = "ms";
  } else {
    scaled = nanos / 1E9; units = "sec";
  }
  printf("%s = %.6f %s +/- %.1f ns\n",
         text, scaled, units, (double) (ofs->toler * mach2nanos));
}

/* Main function */
int
main(int argc, char *argv[])
{
  int argn = 1;
  int continuous = 0, dump = 0, keepgoing = 0;
  int quiet = 0, replay = 0, verbose = 0,  dverbose = 0;
  int err = 0, tterr, ttries, sleepchanged;
  const char *cp;
  char chr;
  sleepofs_t lastsleep, cursleep;

  strncpy(progname, basename(argv[0]), sizeof(progname));
  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 'C': ++continuous; ++iteration; break;
        case 'D': ++dump; break;
        case 'K': ++keepgoing; break;
        case 'q': ++quiet; break;
        case 'R': ++replay; break;
        case 'v': ++verbose; break;
        case 'V': ++dverbose; break;
      }
    }
    ++argn;
  }

  if (verbose && !quiet) printf("%s starting.\n", progname);

  (void) signal(SIGHUP, intsig);
  (void) signal(SIGQUIT, intsig);

  err = setup(verbose && !quiet);

  err |= check_invalid();

  err |= check_boottime(verbose && !quiet);

  get_sleepofs(&lastsleep);
  if (verbose & !quiet) report_sleepofs("  Initial sleep offset", &lastsleep);

  while (!err || keepgoing) {
    err |= report_all_clocks(dump, dverbose, verbose, quiet, replay);
    err |= report_all_clock_compares(&lastsleep,
                                     dump, dverbose, verbose, quiet, replay);
    err |= check_mach_scaling(verbose && !quiet);

    /*
     * Even with relaxed margins, the thread time test occasionally fails
     * (including with Apple's functions), particularly on a heavily loaded
     * system.  So we do a few retries of the retriable errors (designated by
     * the positive return value).  Since this is a fairly rare case, we don't
     * bother to suppress the error messages.
     */
    ttries = THREAD_TEST_TRIES;
    do {
      tterr = check_thread_times(verbose, quiet);
    } while (tterr > 0 && --ttries);
    err |= tterr;

    get_sleepofs(&cursleep);
    sleepchanged = compare_sleepofs(&cursleep, &lastsleep);
    if (sleepchanged) {
      if (sleepchanged < 0) {
        printf("  *** Sleep offset went backwards\n");
        err = 1;
      } else {
        printf("  *** Sleep offset changed\n");
      }
      report_sleepofs("    Old sleep offset", &lastsleep);
      report_sleepofs("    New sleep offset", &cursleep);
      lastsleep = cursleep;
    }

    if (!continuous || stopiter) break;
    ++iteration;
  }

  if (!iteration) {
    if (!quiet) printf("%s %s.\n", progname, err ? "failed" : "passed");
  } else {
    printf("%s %s after %d iterations.\n",
           progname, err ? "failed" : "passed", iteration);
  }
  return err;
}
