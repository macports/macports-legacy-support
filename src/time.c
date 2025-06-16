/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

/* MP support header */
#include "MacportsLegacySupport.h"

#include "compiler.h"

#if __MPLS_LIB_SUPPORT_APPROX_TIME__

#include <mach/mach_time.h>

/*
 * Here we provide a version of mach_approximate_time() which is just a
 * wrapper around the non-approximate version.  Since the only purpose of
 * the "approximate" version is to sacrifice accuracy for greater speed,
 * it's highly unlikely that we could come up with a reasonable way to do
 * better than the fallback approach.
 */

uint64_t mach_approximate_time(void)
{
  return mach_absolute_time();
}

#endif /* __MPLS_LIB_SUPPORT_APPROX_TIME__ */

#if __MPLS_LIB_SUPPORT_CONTINUOUS_TIME__

#include <pthread.h>

#include <mach/mach_time.h>

#include <sys/sysctl.h>
#include <sys/time.h>

/*
 * Continuous time.
 *
 * Unlike mach_absolute_time, mach_continuous_time includes time spent
 * sleeping.  Since pre-10.12 kernels don't directly provide accounting
 * for this, we need to deduce the sleep-time offset from other clock
 * values.  The clocks used for this are:
 *   1) Boottime.  The date/time when the system was first booted.
 *   2) Timeofday.  The current date/time.
 *   3) Mach Absolute Time.  The system "running" time, in mach units.
 * Subtracting #1 from #2 gives us the total time since boot.  Subtracting
 * #3 from that gives us the sleep-time offset (in principle).
 *
 * However, there are a number of issues affecting the accuracy of this
 * approach:
 *   1) The timeofday clock may be adjusted for synchronization purposes,
 *  while the mach time is not.  Assuming the synchronization is to a
 *  correct source, the two will diverge based on the error in the frequency
 *  of the timebase reference clock.
 *   2) Systems prior to 10.12 only track boottime with one-second precision,
 *  leading to an error of up to one second in the #2 - #1 difference.
 *   3) The OS doesn't set boottime as early as it starts counting mach time,
 *  so in the absence of sleeping, the computed sleep offset will typically
 *  be negative.  On x86 systems, this is usually no more than a few seconds,
 *  but on PowerPC systems it's been observed to approach a minute.
 *
 * Since there's not a lot we can do about these issues, at present we simply
 * tolerate #2, and for #3, treat negative sleep offsets as zero.  For #1, we
 * calculate a maximum false offset due to MAX_DRIFT_PPM, and disallow
 * adjustments of less than that.
 *
 * Note that issue #1 represents an ongoing change in the apparent sleep
 * offset.  And since we can only compute the offset local to one program,
 * different programs may disagree as to the sleep offset, and hence on
 * values of the synthesized mach_continuous_time.  There's not much we
 * can do about that.
 *
 * Also note that, since the subsecond portions of timeofday and mach time
 * are the same, they cancel out in the final sleep offset, so that the
 * up to one-second error in the result is the same for all parties.  Hence,
 * although the calculated sleep time may differ from the true value by
 * up to one second (from this source), all programs will agree on that
 * error, and it will not cause a discrepancy across programs.
 *
 * The present implementation only accounts for the "static" sleep offset,
 * i.e. sleeps occuring before the program was launched.  There is no
 * provision for tracking "dynamic" sleep offsets, i.e. sleeps occurring
 * while the program is running.
 *
 * In spite of the above, the code is written to allow for the possibility
 * of recomputing the sleep offset later.  To avoid excessive "churn", it
 * does not update the actual offset unless it's been increased by at least
 * MIN_SLEEP_OFFSET_ADVANCE seconds, or the drift-based limit if larger.
 * Since the initial offset is zero, this also has the effect of excluding
 * negative offsets as noted above.
 *
 * If the first "raw" offset is negative, it's retained to use as a baseline
 * for any subsequent adjustments (not currently implemented), thereby removing
 * the "boottime delay" from any subsequent adjustments.
 *
 * Unlike the mach scale factor setup, attempting to set up the sleep offset
 * simultaneously from multiple threads can cause trouble, so we protect
 * the setup code with a mutex.
 *
 * To maximize consistency, we use a "constructor" function to initialize
 * the sleep offset at program launch, rather than waiting for the first
 * use.  This means that programs launched at about the same time should
 * have close agreement on the offset.  An added benefit is that it avoids
 * the additional slowness on the first relevant call.  And since it needs
 * the mach scale factor, it also eliminates most of the first-time delay
 * in the other calls using that factor.
 */

#define MIN_SLEEP_OFFSET_ADVANCE 5
#define MAX_DRIFT_PPM 100

typedef struct sleepofs_info_s {
  struct timeval boottime;
  struct timeval timeofday;
  uint64_t mach_before;
  uint64_t mach_after;
  uint64_t mach_diff;
} sleepofs_info_t;

static int sleep_offset_valid = 0;
static uint64_t sleep_offset = 0;
static int64_t raw_offset = 0, first_offset = 0;
static sleepofs_info_t sleep_info_raw, sleep_info = {.mach_before = 0};
static pthread_mutex_t sleepofs_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Get the system boot time, via sysctl.  The comm-page method of obtaining
 * this faster and with better resolution didn't appear until 10.12, where
 * this code is no longer relevant.
 */
static int
get_boottime(struct timeval *bt)
{
  size_t bt_len = sizeof(*bt);

  int bt_mib[] = {CTL_KERN, KERN_BOOTTIME};
  size_t bt_miblen = sizeof(bt_mib) / sizeof(bt_mib[0]);

  bt->tv_usec = 0;  /* In case OS doesn't store it */
  return sysctl(bt_mib, bt_miblen, bt, &bt_len, NULL, 0);
}

/*
 * Get timeofday/mach_time pair.
 *
 * This obtains the timeofday at its next change, sandwiched by a pair
 * of mach_absolute_time reads.
 */
static int
get_todmach(sleepofs_info_t *si)
{
  struct timeval tv1, tv2;
  uint64_t mt1, mt2;

  if (gettimeofday(&tv1, NULL)) return -1;
  mt2 = mach_absolute_time();
  do {
    mt1 = mt2;
    if (gettimeofday(&tv2, NULL)) return -1;
    mt2 = mach_absolute_time();
  } while (tv2.tv_usec == tv1.tv_usec);

  si->timeofday = tv2;
  si->mach_before = mt1;
  si->mach_after = mt2;
  /* Round up and add one unit to mach diff */
  si->mach_diff = (mt2 - mt1 + 1 + 2) / 2;

  return 0;
}

/*
 * Get the parameters for calculating the sleep offset.
 *
 * Although boottime should be constant in all pre-10.12 systems, we still
 * verify that it doesn't change during the collection of the tod/mach pair.
 * Within that, we obtain the "tightest" of five tod/mach samples.
 */
static int
get_sleepofs_info(sleepofs_info_t *si)
{
  int tries;
  sleepofs_info_t si2;

  if (get_boottime(&si->boottime)) return -1;
  si2.boottime = si->boottime;

  while(1) {
    if (get_todmach(si)) return -1;

    tries = 5;  /* Get best of 5 samples */
    while (--tries) {
      if (get_todmach(&si2)) return -1;
      if (si2.mach_diff < si->mach_diff) *si = si2;
    }

    if (get_boottime(&si2.boottime)) return -1;
    if (si2.boottime.tv_sec == si->boottime.tv_sec
        && si2.boottime.tv_usec == si->boottime.tv_usec) break;
    *si = si2;
  }

  return 0;
}

/*
 * Compute the sleep offset.  Do nothing on failure, leaving the offset as is.
 */
static int get_mach_scale(void);
static int64_t tvdiff2mach(const struct timeval *tv1,
                           const struct timeval *tv2);

static void get_sleep_offset(void)
{
  int64_t toddiff, offset, minsleepadj, maxdrift;
  sleepofs_info_t si;
  static const struct timeval tv5a = {MIN_SLEEP_OFFSET_ADVANCE, 0},
                              tv5b = {0, 0};

  if (get_mach_scale()) return;
  if (get_sleepofs_info(&si)) return;

  toddiff = tvdiff2mach(&si.timeofday, &si.boottime);
  /* boottime later than tod is garbage */
  if (toddiff < 0) {
    sleep_offset_valid = 1;  /* It's permanent garbage, so don't retry this */
    return;
  }
  offset = toddiff - (si.mach_before + si.mach_after) / 2;
  minsleepadj = tvdiff2mach(&tv5a, &tv5b);
  maxdrift = (si.mach_before - sleep_info.mach_before)
             / (1000000 / MAX_DRIFT_PPM);
  if (maxdrift > minsleepadj) minsleepadj = maxdrift;

  if (pthread_mutex_lock(&sleepofs_lock)) return;

  raw_offset = offset;
  if (!first_offset && offset < 0) first_offset = offset;
  sleep_info_raw = si;

  if (offset - first_offset > sleep_offset + minsleepadj) {
    sleep_offset = offset - first_offset;
    sleep_info = si;
  }

  /* The offset is now valid, whether we decided to change it or not. */
  sleep_offset_valid = 1;

  (void) pthread_mutex_unlock(&sleepofs_lock);
}

/*
 * Initialize the sleep offset at program launch.
 *
 * If this fails for some reason, we'll try again at first use.
 */
static void __attribute__((constructor))
startup_sleep_offset(void)
{
  get_sleep_offset();
}

/*
 * For some as-yet-undetermined reason, the sleep offset is causing trouble
 * in ppc64 builds.  So for now, we avoid it on ppc64.
 */

#ifndef __ppc64__

uint64_t mach_continuous_time(void)
{
  uint64_t mach_time;

  if (MPLS_SLOWPATH(!sleep_offset_valid)) get_sleep_offset();
  mach_time = mach_absolute_time();
  return mach_time + sleep_offset;
}

uint64_t mach_continuous_approximate_time(void)
{
  uint64_t mach_time;

  if (MPLS_SLOWPATH(!sleep_offset_valid)) get_sleep_offset();
  mach_time = mach_approximate_time();
  return mach_time + sleep_offset;
}

#else /* __ppc64__ */

uint64_t mach_continuous_time(void)
{
  return mach_absolute_time();
}

uint64_t mach_continuous_approximate_time(void)
{
  return mach_approximate_time();
}

#endif /* __ppc64__ */

#endif /* __MPLS_LIB_SUPPORT_CONTINUOUS_TIME__ */

#if __MPLS_LIB_SUPPORT_GETTIME__

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_time.h>
#include <mach/thread_act.h>

/* Constants for scaling time values */
#define BILLION32 1000000000U
#define BILLION64 1000000000ULL

/*
 * CLOCK_MONOTONIC
 *
 * Apple's implementation of CLOCK_MONOTONIC involves subtracting boottime
 * from the current time of day.  That relies on boottime's having step
 * adjustments applied to it, so that they cancel out in the difference.
 * But aside from the fact that corrupting boottime for this purpose is
 * a kludge, this method doesn't even work at all prior to 10.12, since
 * pre-10.12 kernels don't adjust boottime in settimeofday().  Also, prior
 * to 10.12, boottime only had one-second resolution, so the scheme
 * wouldn't work for adjustments less than one second.  Hence, the Apple
 * algorithm for CLOCK_MONOTONIC is completely invalid prior to 10.12.
 *
 * Aside from the mach_absolute_time() variants used for the RAW & UPTIME
 * clocks, the only other non-timeofday clock is the mach SYSTEM_CLOCK.
 * But this is just a slow and more complicated wrapper around
 * mach_absolute_time(), with scaling to nanoseconds, so it's not helpful
 * for this purpose.
 *
 * The net result is that CLOCK_MONOTONIC on pre-10.12 systems can't be made
 * any different from CLOCK_MONOTONIC_RAW without losing the mandatory
 * monotonicity property.  Note that neither following adjtime() slewing
 * nor counting during sleeps are mandatory properties of CLOCK_MONOTONIC
 * in general, though they happen to be true of the 10.12+ Apple version.
 *
 * For maximum consistency with Apple's microsecond-resolution implementation,
 * we limit the resolution of our implementation to microseconds.  Although
 * we could use an alternate scale factor to get microseconds directly, this
 * reduces accuracy, and it isn't worth worrying too much about the speed of
 * this function, which is almost never the appropriate choice anyway.
 */

/*
 * Mach timebase scaling
 *
 * Many time types use "mach_time", which is a timescale based on arbitrary
 * units that can be converted to nanoseconds via a separately provided
 * scale factor.  Nothing in the Apple documentation of this function
 * indicates that this scale factor should be constant, and Apple's own
 * code for these functions fetches it on every call, but in 10.12+, that
 * function itself caches the scale factor.  Because it's 10.12+, it's
 * known not to be applied to PowerPC, but if the scale factor for PowerPC
 * is ever updated at all after the initial boot (which is highly unlikely),
 * it would only change it by a small amount due to thermal variations, so we
 * assume that cacheing it is safe.
 *
 * The scale factor is provided as a rational number for maximum accuracy,
 * with a 32-bit numerator and a 32-bit denominator.  The observed values
 * on a few systems are:
 *   PowerPC:  1000000000 / <frequency in Hz>
 *   x86:               1 / 1
 *   arm64 (M1):      125 / 3
 * In the x86 case, the true scaling happens at a lower level, with mach_time
 * always being in nanoseconds.  The numbers in the arm64 case are sufficiently
 * "round" that it's clear that they're based on a nominal value, rather than a
 * measured value.  Only PowerPC actually measures the frequency, with a
 * value that changes slightly (on the order of 2ppm) based on temperature.
 *
 * To actually realize the full accuracy of the rational representation, it's
 * necessary to compute nanoseconds as:
 *   nanoseconds = (mach_time * numerator) / denominator
 * However, with some scale-factor values (e.g., PowerPC), the intermediate
 * result easily overflows 64 bits.  Overflow can be avoided by using:
 *   nanoseconds = mach_time * (numerator / denominator)
 * But this results in a significant inaccuracy in the PowerPC case.
 * Apple's code takes the former approach, but it's only present in OS
 * versions that don't support PowerPC, and neither the x86 nor the arm64
 * values are overflow-prone.
 *
 * The only way to get maximum accuracy while avoiding overflow is to use
 * double-precision arithmetic (or floating point, but that's currently being
 * avoided).  The conceptually straightforward approach would be to do
 * the same multiply-first calculation as above, but with double-precision
 * multiply and divide.  But double-precision divide is messy and slow, so
 * it's attractive to consider a multiply-only approach.
 *
 * Another possibility would be to scale the upper and lower halves of the
 * mach time separately and combine the results, but this would require
 * incorporating the remainder from the upper divide into the lower divide,
 * again making things messy and slow, and again suggesting the multiply-only
 * approach.
 *
 * With a 64-bit normalized multiplier and a corresponding shift (effectively
 * software floating-point), the accuracy would be extreme, but that would
 * require a variable double-precision shift in addition to the double-precision
 * multiply.  If we instead use an unnormalized multiplier chosen for the
 * desired result scale, then no shifting is needed (other than some 32-bit
 * shifts that aren't really shifts).  With the actual observed scale factors,
 * the maximum error from this approach is on the order of a couple of parts
 * per trillion.
 *
 * In this approach, the most convenient scaling is with a 64-bit multiplier
 * whose binary point is in the middle, i.e. a 32-bit integer part and a 32-bit
 * fractional part.  Multiplying this by the 64-bit mach_time yields a 128-bit
 * product whose middle 64 bits are the result in nanoseconds.  This scale
 * factor is easily computed as:
 *   scale = (numerator << 32) / denominator
 * or, with rounding:
 *   scale = ((numerator << 32) + denominator / 2) / denominator
 * On x86, the scale becomes 1.0, which we check for when used to avoid the
 * superfluous multiply.
 *
 * Although a fully-normalized multipler is inconvenient, we can improve the
 * accuracy some with a small and simple tweak.  Since the maximum numerator
 * fits in 30 bits, we can shift it left an additional two bits when computing
 * the multiplier, producing a final nanosecond result shifted by two bits.
 * To get the full 64-bit nanosecond result, we'd need to right shift the high
 * 96 bits of the 128-bit product by two bits.  But since it takes over 143
 * years of uptime to overflow 62 bits, we can skip the upper part and just
 * right shift the middle 64 bits of the product.
 *
 * Out of maximum paranoia, we check for the case where the numerator doesn't
 * fit in 30 bits, and apply the left shift *after* the divide, to get the
 * expected scale.  We don't expect this code to be reached in practice.
 *
 * For the clock_gettime() case, the ultimate result is a timespec, with
 * separate seconds and nanoseconds.  The straightforward mutiply-only
 * approach doesn't work out so well in this case, either in range or in
 * error magnitude, so we just compute nanoseconds as in the former case,
 * and then divide to get seconds.  To get the nanosecond remainder, we
 * multiply back and subtract, which is faster than using the modulo operator,
 * and none of the *div() functions provdes the needed mixed-precision
 * operation needed here.
 *
 * The other use of mach_time scaling is for clock_getres(), where the
 * scale factor actually represents the resolution of all clocks based on
 * mach time.  This function isn't time-critical, but for consistency
 * we just use the same flow as the other cases, with cacheing.
 *
 * The primary cached scale factors in all cases are the derived factors,
 * not the OS-provided mach scale.  But for maximum consistency, we also
 * share a single cached copy of the mach scale across all uses.
 *
 * Apple's code has a somewhat convoluted structure in order to do the
 * mach scaling setup prior to obtaining the mach time value, presumably
 * to ensure that the time obtained is as close as possible to the function's
 * return, even though the scale factor is cached.  It also avoids obtaining
 * the scale factor for clocks that don't need it, in spite of the cacheing.
 * Here we just always do the setup first, regardless of clock type, but
 * defer the reporting of any related error until the need is known.
 */

#define EXTRA_SHIFT 2
#define HIGH_SHIFT (32 + EXTRA_SHIFT)
#define HIGH_BITS (64 - HIGH_SHIFT)
#define NUMERATOR_MASK (~0U << HIGH_BITS)
#define NULL_SCALE (1ULL << HIGH_SHIFT)

/* The cached mach_time scale factors */
static mach_timebase_info_data_t mach_scale = {0};
static uint64_t mach_mult = 0;
static struct timespec res_mach = {0, 0};

/* And the fixed microsecond resolution for timeval-based clocks */
static struct timespec res_micros = {0, 1000};

/* Obtain the mach_time scale factor if needed, or return an error */
static int
get_mach_scale(void)
{
  if (mach_scale.numer) return 0;
  if (mach_timebase_info(&mach_scale) != KERN_SUCCESS) {
    /* On failure, make sure resulting scale is 0 */
    mach_scale.numer = 0;
    mach_scale.denom = 1;
    return -1;
  }
  return 0;
}

/* Set up the mach->nanoseconds multiplier, or return an error */
static int
setup_mach_mult(void)
{
  int ret = get_mach_scale();

  /* Set up main multiplier (0 if error getting scale) */
  if (!(mach_scale.numer & NUMERATOR_MASK)) {
    mach_mult = (((uint64_t) mach_scale.numer << HIGH_SHIFT)
                 + mach_scale.denom / 2) / mach_scale.denom;
  } else {
    mach_mult = ((((uint64_t) mach_scale.numer << 32)
                 + mach_scale.denom / 2) / mach_scale.denom) << EXTRA_SHIFT;
  }

  /* Also set up resolution as nanos/count rounded up */
  res_mach.tv_nsec = (mach_mult + (NULL_SCALE - 1)) >> HIGH_SHIFT;

  return ret;
}

#define MASK64LOW 0xFFFFFFFFULL

/*
 * 64x64->128 multiply, returning middle 64
 *
 * This code has been verified with a floating-zeroes/ones test, comparing
 * the results to Python's built-in multiprecision arithmetic.
 */
static inline uint64_t
mmul64(uint64_t a, uint64_t b)
{
  /* Split the operands into halves */
  uint32_t a_hi = a >> 32, a_lo = a;
  uint32_t b_hi = b >> 32, b_lo = b;
  uint64_t high, mid1, mid2, low;

  /* Compute the four cross products */
  low = (uint64_t) a_lo * b_lo;
  mid1 = (uint64_t) a_lo * b_hi;
  mid2 = (uint64_t) a_hi * b_lo;
  high = (uint64_t) a_hi * b_hi;

  /* Fold the results (must be in carry-propagation order) */
  mid1 += (mid2 & MASK64LOW) + (low >> 32);
  high += (mid1 >> 32) + (mid2 >> 32);  /* Shifts must precede add */

  /* Combine and return the two middle chunks */
  return (high << 32) + (mid1 & MASK64LOW);
}

/* Convert mach units to nanoseconds */
static inline uint64_t
mach2nanos(uint64_t mach_time)
{
  /* If 1:1 scaling (x86), return as is */
  if (mach_mult == NULL_SCALE) return mach_time;

  /* Otherwise, return appropriately scaled value */
  return mmul64(mach_time, mach_mult) >> EXTRA_SHIFT;
}

/* Convert nanoseconds to timespec */
static inline void
nanos2timespec(uint64_t nanos, struct timespec *ts)
{
  uint64_t secs;
  uint32_t lownanos, lowsecs, nanorem;

  /* Divide nanoseconds to get seconds */
  secs = nanos / BILLION32;

  /*
   * Multiply & subtract (all 32-bit) to get nanosecond remainder.
   *
   * This is more efficient than using the '%' operator on all platforms,
   * and there's no version of *div() for a 64-bit dividend and 32-bit
   * divisor.  Since the divisor, and hence the remainder, are known to
   * fit in 32 bits, the entire computation can be done in 32 bits.
   */
  lownanos = nanos; lowsecs = secs;
  nanorem = lownanos - lowsecs * BILLION32;

  /* Return values as a timespec */
  ts->tv_sec = secs; ts->tv_nsec = nanorem;
}

/* Convert mach units to timespec */
static inline void
mach2timespec(uint64_t mach_time, struct timespec *ts)
{
  nanos2timespec(mach2nanos(mach_time), ts);
}

/*
 * Convert timeval diff to mach units.
 *
 * Since this is only used for the sleep offset setup, we do it the easy
 * way and use floating-point.
 *
 * If the tv_sec fields are 32-bit, we force unsigned interpretation
 * to get around a possible Y2038 isue.
 */
static int64_t
tvdiff2mach(const struct timeval *tv1, const struct timeval *tv2)
{
  double tvdiff;

  if (sizeof(tv1->tv_sec) == sizeof(int32_t)) {
    tvdiff = ((uint32_t) tv1->tv_sec - (uint32_t) tv2->tv_sec) * 1E9;
  } else {
    tvdiff = (tv1->tv_sec - tv2->tv_sec) * 1E9;
  }
  tvdiff += (tv1->tv_usec - tv2->tv_usec) * 1000.0;
  return tvdiff * mach_scale.denom / mach_scale.numer;
}

/*
 * Get the best available thread time, using the syscall on 10.10+,
 * but falling back to thread_info() on <10.10.
 *
 * In the latter case, a thread which has just started may not yet have
 * nonzero usage (in microsecond resolution).  This is not a problem in
 * the timespec case, but is a problem in the nanosecond case, since a zero
 * value indicates an error.  To get around this, we report a non-error
 * zero nanosecond result as one nanosecond.  It's impossible for this to
 * exceed any later non-error result, hence it's monotonic.
 *
 * Out of extreme paranoia, we apply the same treatment to the timespec
 * case, so that using it after using the nanosecond version can't show
 * a backstep.
 */

#if __MPLS_TARGET_OSVER < 101000

/* Common thread usage code */
static int
get_thread_usage(thread_basic_info_data_t *info)
{
  int ret;
  mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
  thread_port_t thread = mach_thread_self();

  ret = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) info, &count);
  mach_port_deallocate(mach_task_self(), thread);
  return ret;
}

/* Get the CPU usage of the current thread, in nanoseconds */
static inline uint64_t
get_thread_usage_ns(void)
{
  thread_basic_info_data_t info;
  uint64_t nanos;

  if (get_thread_usage(&info)) return 0;

  nanos = (info.user_time.seconds + info.system_time.seconds) * BILLION64
          + (info.user_time.microseconds + info.system_time.microseconds)
            * 1000;
  return nanos ? nanos : 1;
}

/* Same but returning as timespec */
static inline int
get_thread_usage_ts(struct timespec *ts)
{
  thread_basic_info_data_t info;

  if (get_thread_usage(&info)) return -1;

  ts->tv_sec = info.user_time.seconds + info.system_time.seconds;
  ts->tv_nsec = (info.user_time.microseconds + info.system_time.microseconds)
                * 1000;
  if (ts->tv_nsec >= BILLION32) {
    ++ts->tv_sec;
    ts->tv_nsec -= BILLION32;
  }

  if (!ts->tv_sec && !ts->tv_nsec) ts->tv_nsec = 1;
  return 0;
}

#define HIRES_THREAD_TIME 0

#else /* __MPLS_TARGET_OSVER >= 101000 */

/* Get the CPU usage of the current thread via syscall, in nanoseconds. */
static inline uint64_t
get_thread_usage_ns(void)
{
  uint64_t mach_time = syscall(SYS_thread_selfusage);

  return mach2nanos(mach_time);
}

/* Same but returning as timespec */
static inline int
get_thread_usage_ts(struct timespec *ts)
{
  uint64_t mach_time = syscall(SYS_thread_selfusage);

  mach2timespec(mach_time, ts);
  return mach_time ? 0 : -1;
}

#define HIRES_THREAD_TIME 1

#endif /* __MPLS_TARGET_OSVER >= 101000 */

/* Now the actual public functions */

uint64_t
clock_gettime_nsec_np(clockid_t clk_id)
{
  struct timeval tod;
  struct rusage ru;
  uint64_t mach_time;

  /* Set up mach scaling early, whether we need it or not. */
  if (MPLS_SLOWPATH(!mach_mult)) setup_mach_mult();

  switch (clk_id) {

  case CLOCK_REALTIME:
    if (gettimeofday(&tod, NULL)) return 0;
    return tod.tv_sec * BILLION64 + tod.tv_usec * 1000;

  case CLOCK_PROCESS_CPUTIME_ID:
    if (getrusage(RUSAGE_SELF, &ru)) return 0;
    return (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * BILLION64
           + (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) * 1000;

  case CLOCK_THREAD_CPUTIME_ID:
    return get_thread_usage_ns();

  case CLOCK_MONOTONIC:
    mach_time = mach_continuous_time();
    return mach2nanos(mach_time) / 1000 * 1000;  /* Quantize to microseconds */

  case CLOCK_MONOTONIC_RAW:
    mach_time = mach_continuous_time();
    break;

  case CLOCK_MONOTONIC_RAW_APPROX:
    mach_time = mach_continuous_approximate_time();
    break;

  case CLOCK_UPTIME_RAW:
    mach_time = mach_absolute_time();
    break;

  case CLOCK_UPTIME_RAW_APPROX:
    mach_time = mach_approximate_time();
    break;

  default:
    errno = EINVAL;
    return 0;
  }

  /* Return scaled mach_time (0 if scale unobtained) */
  return mach2nanos(mach_time);
}

int
clock_gettime(clockid_t clk_id, struct timespec *ts)
{
  int ret, mserr = 0;
  struct timeval tod;
  struct rusage ru;
  uint64_t mach_time, nanos;

  /* Set up mach scaling early, whether we need it or not. */
  if (MPLS_SLOWPATH(!mach_mult)) mserr = setup_mach_mult();

  switch (clk_id) {

  case CLOCK_REALTIME:
    ret = gettimeofday(&tod, NULL);
    ts->tv_sec = tod.tv_sec; ts->tv_nsec = tod.tv_usec * 1000;
    return ret;

  case CLOCK_PROCESS_CPUTIME_ID:
    ret = getrusage(RUSAGE_SELF, &ru);
    timeradd(&ru.ru_utime, &ru.ru_stime, &ru.ru_utime);
    TIMEVAL_TO_TIMESPEC(&ru.ru_utime, ts);
    return ret;

  case CLOCK_THREAD_CPUTIME_ID:
    return get_thread_usage_ts(ts);

  case CLOCK_MONOTONIC:
    mach_time = mach_continuous_time();
    nanos = mach2nanos(mach_time) / 1000 * 1000;  /* Quantize to microseconds */
    nanos2timespec(nanos, ts);
    return mserr;

  case CLOCK_MONOTONIC_RAW:
    mach_time = mach_continuous_time();
    break;

  case CLOCK_MONOTONIC_RAW_APPROX:
    mach_time = mach_continuous_approximate_time();
    break;

  case CLOCK_UPTIME_RAW:
    mach_time = mach_absolute_time();
    break;

  case CLOCK_UPTIME_RAW_APPROX:
    mach_time = mach_approximate_time();
    break;

  default:
    errno = EINVAL;
    return -1;
  }

  /* Convert to timespec & return (error if scale couldn't be obtained) */
  mach2timespec(mach_time, ts);
  return mserr;
}

int
clock_getres(clockid_t clk_id, struct timespec *res)
{
  int mserr = 0;

  /* Set up mach scale factor, whether we need it or not. */
  if (MPLS_SLOWPATH(!res_mach.tv_nsec)) mserr = setup_mach_mult();

  switch (clk_id) {

  /* Everything based on timeval has microsecond resolution. */
  case CLOCK_REALTIME:
  case CLOCK_PROCESS_CPUTIME_ID:
#if !HIRES_THREAD_TIME
  case CLOCK_THREAD_CPUTIME_ID:
#endif
  case CLOCK_MONOTONIC:  /* Forced microsecond resolution */
    *res = res_micros;
    return 0;

  /* Everything based on mach_time has mach resolution. */
  case CLOCK_MONOTONIC_RAW:
  case CLOCK_MONOTONIC_RAW_APPROX:
  case CLOCK_UPTIME_RAW:
  case CLOCK_UPTIME_RAW_APPROX:
#if HIRES_THREAD_TIME
  case CLOCK_THREAD_CPUTIME_ID:
#endif
    break;

  default:
    errno = EINVAL;
    return -1;
  }

  /* Return proper scale (error if scale couldn't be obtained  */
  *res = res_mach;
  return mserr;
}

int
clock_settime(clockid_t clk_id, const struct timespec *ts)
{
  struct timeval tv;

  switch (clk_id) {

  case CLOCK_REALTIME:
    tv.tv_sec = ts->tv_sec;
    tv.tv_usec = ts->tv_nsec / 1000;
    return settimeofday(&tv, NULL);

  default:
    errno = EINVAL;
    return -1;
  }
}

#endif /* __MPLS_LIB_SUPPORT_GETTIME__ */

#if __MPLS_LIB_SUPPORT_TIMESPEC_GET__

#include <time.h>

/* In case built with SDK that's sparing with the definition */
#ifndef TIME_UTC
#define TIME_UTC	1	/* time elapsed since epoch */
#endif

int timespec_get(struct timespec *ts, int base)
{
  switch (base) {
    case TIME_UTC:
      if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
        return 0;
      }
      return base;

    default:
      return 0;
  }
}

#endif /* __MPLS_LIB_SUPPORT_TIMESPEC_GET__ */
