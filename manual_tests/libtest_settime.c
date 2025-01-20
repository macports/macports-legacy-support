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
 * This is a test of the clock_settime() function.  Because it requires
 * invasively altering the system time, it's a manual test that must be
 * run as root.
 *
 * We take a few precautions to minimize the potential trouble:
 *   1) The time adjustment is only 100ms, which is less than the typical
 *  NTP threshold for making step adjustments.
 *   2) We attempt to almost immediately undo the adjustment.
 *   3) The adjustment is first forward and then backward, so if the test
 *  dies in midstream, we're left with the less troublesome forard step.
 *   4) We set our priority to the maximum, and launch as many threads
 *  as there are CPUs, to mostly lock out other programs (briefly).
 *   5) We begin with 100ms sleep, to start with a fresh quantum and
 *  minimize the chance of a reschedule in midstream.
 */

#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#define SLEEP_MS 500
#define TIME_BUMP_MS 100

/* Fudge factor in validity checks */
#define FUDGE_NS 1000

#define TARGET_PRIO -20

#define MILLION 1000000LL
#define BILLION (MILLION * 1000)

typedef long long nstime_t;

typedef struct info_s {
  nstime_t init_raw;
  nstime_t init_real;
  nstime_t first_set;
  nstime_t first_real;
  nstime_t middle_raw;
  nstime_t middle_real;
  nstime_t second_set;
  nstime_t second_real;
  nstime_t final_raw;
} info_t;

static nstime_t
timespec2ns(const struct timespec *ts)
{
  return ts->tv_sec * BILLION + ts->tv_nsec;
}

static struct timespec
ns2timespec(nstime_t nsec)
{
  struct timespec ts;

  ts.tv_sec = nsec / BILLION;
  ts.tv_nsec = nsec % BILLION;
  return ts;
}

static int
clock_getns(clockid_t clock_id, nstime_t *nsp)
{
  struct timespec ts;

  if (clock_gettime(clock_id, &ts) < 0) return errno;
  *nsp = timespec2ns(&ts);
  return 0;
}

static int
clock_setns(nstime_t nsec)
{
  struct timespec ts = ns2timespec(nsec);

  if (clock_settime(CLOCK_REALTIME, &ts) < 0) return -1;
  return 0;
}

static int
do_test(info_t *tp)
{
  int err, tverr;
  nstime_t delta;
  struct timeval orig_tv;

  /* First warm up the clocks */
  (void) clock_getns(CLOCK_MONOTONIC_RAW, &tp->init_raw);
  (void) clock_getns(CLOCK_REALTIME, &tp->init_raw);

  /* Get us a fresh quantum */
  (void) usleep(SLEEP_MS * 1000);

  /* Save original gettimeofday() time */
  tverr = gettimeofday(&orig_tv, NULL);

  /* Get both real and raw times */
  if (clock_getns(CLOCK_MONOTONIC_RAW, &tp->init_raw)) return errno;
  if (clock_getns(CLOCK_REALTIME, &tp->init_real)) return errno;

  /* Adjust clock forward */
  tp->first_set = tp->init_real + TIME_BUMP_MS * MILLION;
  if (clock_setns(tp->first_set)) return errno;

  /* While clock is futzed, try to undo it on any error */
  do {

    /* Capture times from the middle */
    if ((err = clock_getns(CLOCK_REALTIME, &tp->first_real))) break;
    if ((err = clock_getns(CLOCK_MONOTONIC_RAW, &tp->middle_raw))) break;
    if ((err = clock_getns(CLOCK_REALTIME, &tp->middle_real))) break;

    /* Adjust clock backward, can't fix it if it doesn't work */
    tp->second_set = tp->middle_real - TIME_BUMP_MS * MILLION;
    err = clock_setns(tp->second_set);

  } while (0);

  /* If anything went wrong, try to put the clock back to the start */
  if (err) {
    err = errno;
    if (!tverr) (void) settimeofday(&orig_tv, NULL);
    return err;
  }

  /* Otherwise, finish up with a couple more captures */
  if (clock_getns(CLOCK_REALTIME, &tp->second_real)) return errno;
  if (clock_getns(CLOCK_MONOTONIC_RAW, &tp->final_raw)) return errno;

  /* Just to be safe, restore the time via settimeofday() */
  if (!tverr) {
    delta = tp->final_raw - tp->init_raw;
    orig_tv.tv_sec += delta / BILLION;
    orig_tv.tv_usec += delta % BILLION / 1000;
    if (orig_tv.tv_usec >= MILLION) {
      ++orig_tv.tv_sec; orig_tv.tv_usec -= MILLION;
    }
    (void) settimeofday(&orig_tv, NULL);
  }

  /* All's good if here */
  return 0;
}

#define PRINT_TIME(ptr,name) printf("  " #name " = %lld ns\n", \
    (long long) ptr->name)

static void
print_times(info_t *tp) {
  PRINT_TIME(tp, init_raw);
  PRINT_TIME(tp, init_real);
  PRINT_TIME(tp, first_set);
  PRINT_TIME(tp, first_real);
  PRINT_TIME(tp, middle_raw);
  PRINT_TIME(tp, middle_real);
  PRINT_TIME(tp, second_set);
  PRINT_TIME(tp, second_real);
  PRINT_TIME(tp, final_raw);
}

static void *
thread_spin(void *arg)
{
  while (1) ;
  return NULL;
}

static void
unhog_cpus(pthread_t threads[], int nthreads)
{
  while (--nthreads >= 0) {
    (void) pthread_cancel(threads[nthreads]);
  }
}

static int
hog_cpus(pthread_t threads[], int nthreads)
{
  int i, err;

  for (i = 0; i < nthreads; ++i) {
    if (pthread_create(&threads[i], NULL, thread_spin, NULL)) {
      err = errno;
      unhog_cpus(threads, i);
      return err;
    }
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  int verbose = 0;
  int nc_mib[] = {CTL_HW, HW_NCPU};
  size_t nc_miblen = sizeof(nc_mib) / sizeof(nc_mib[0]);
  int ncpus;
  size_t ncpus_sz = sizeof(ncpus);
  int orig_prio;
  pthread_t *threads;
  int err;
  nstime_t dur1, dur2, diff1, diff2;
  info_t info = {0};

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", basename(argv[0]));

  if (sysctl(nc_mib, nc_miblen, &ncpus, &ncpus_sz, NULL, 0) < 0) {
    perror("sysctl for ncpus failed");
    return 1;
  }
  if (verbose) printf("Number of CPUs = %d\n", ncpus);

  if (!(threads = calloc(ncpus, sizeof(pthread_t)))) {
    perror("unable to allocate thread array");
    return 1;
  }

  errno = 0;
  orig_prio = getpriority(PRIO_PROCESS, 0);
  if (orig_prio == -1 && errno) {
    perror("can't get current priority");
    free(threads);
    return 1;
  }
  if (verbose) printf("Initial priority = %d\n", orig_prio);

  if (setpriority(PRIO_PROCESS, 0, TARGET_PRIO) < 0) {
    perror("Unable to set priority");
    fprintf(stderr, "Continuing anyway (will probably fail)\n");
  }

  /* Try to lock out everyone else while we screw with the clock */
  if ((err = hog_cpus(threads, ncpus))) {
    fprintf(stderr, "Unable to hog CPU with threads: %s\n", strerror(err));
    free(threads);
    return 1;
  }

  /* Do the test */
  err = do_test(&info);

  /* Undo our hogging */
  unhog_cpus(threads, ncpus);

  /* Clean up before reporting */
  (void) setpriority(PRIO_PROCESS, 0, orig_prio);
  free(threads);

  if (err) {
    printf("Error encountered: %s\n", strerror(err));
    printf("Dumping partial results:\n");
    print_times(&info);
    return 1;
  }

  /* See if the results are plausible */
  dur1 = info.middle_raw - info.init_raw;
  dur2 = info.final_raw - info.middle_raw;
  diff1 = info.first_real - info.first_set;
  diff2 = info.second_real - info.second_set;

  /* First check raw monotonicity (very unlikely wrong) */
  if (dur1 < 0) {
    printf("Middle raw %lld < init raw %lld\n",
           info.middle_raw, info.init_raw);
    err = 1;
  }
  if (dur2 < 0) {
    printf("Final raw %lld < middle raw %lld\n",
           info.final_raw, info.middle_raw);
    err = 1;
  }

  /* Now see if clock setting had reasonable effect */
  if (llabs(diff1) > dur1 + FUDGE_NS) {
    printf("First set/read delta was %lld ns, bracketed by %lld ns\n",
           diff1, dur1);
    err = 1;
  }
  if (llabs(diff2) > dur2 + FUDGE_NS) {
    printf("Second set/read delta was %lld ns, bracketed by %lld ns\n",
           diff2, dur2);
    err = 1;
  }

  if (verbose) {
    print_times(&info);
    printf("Total ns = %lld\n", info.final_raw - info.init_raw);
  }

  printf("%s %s.\n", basename(argv[0]), err ? "failed" : "passed");
  return err;
}
