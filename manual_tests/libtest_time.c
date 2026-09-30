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
 * This is a rewrite of the C++ version in C, to avoid some Apple SDK issues.
 *
 * It's now been replaced by the new fully automated test, but is retained as
 * a manual test (which it always was, anyway).
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef unsigned long long ULL_t;

static ULL_t kSecondsToNanos = 1000ULL * 1000ULL * 1000ULL;

static inline int
get_time(clockid_t clk_id, ULL_t *val)
{
  int ret;
  struct timespec ts = {0, 0};

  ret = clock_gettime(clk_id, &ts);
  if (ret) perror("clock_gettime failed");

  *val = ts.tv_sec * kSecondsToNanos + ts.tv_nsec;

  return ret;
}

static inline void
show_res(clockid_t clk_id)
{
  int ret;
  struct timespec ts = {0, 0};

  ret = clock_getres(clk_id, &ts);
  if (ret) perror("clock_getres failed");

  printf("%d %lld %ld\n", ret, (long long) ts.tv_sec, ts.tv_nsec);
}

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *
test_thread_sleep(void *arg)
{
  int delay = *(int *)arg;
  int c = 0;
  ULL_t was, now;

  while (++c < 10) {
    assert(get_time(CLOCK_THREAD_CPUTIME_ID, &was) == 0
           && "CLOCK_THREAD_CPUTIME_ID failed");
    usleep(delay);
    assert(get_time(CLOCK_THREAD_CPUTIME_ID, &now) == 0
           && "CLOCK_THREAD_CPUTIME_ID failed");

    assert(pthread_mutex_lock(&lock) == 0 && "pthread_mutex_lock failed");
    printf("[sleep for %dus] consumed thread CPU time: %llu ns\n",
           delay, (now - was));
    assert(pthread_mutex_unlock(&lock) == 0 && "pthread_mutex_unlock failed");
  }

  return NULL;
}

static void *
test_blackhole_thread(void *arg)
{
  int cycles = *(int *)arg;
  int c = 0;
  ULL_t was, now;
  int i = 0;
  int cc = 0;

  while (++c < 10) {
    assert(get_time(CLOCK_THREAD_CPUTIME_ID, &was) == 0
           && "CLOCK_THREAD_CPUTIME_ID failed");
    while (++cc < cycles) i = cc * cc % c;
    assert(get_time(CLOCK_THREAD_CPUTIME_ID, &now) == 0
           && "CLOCK_THREAD_CPUTIME_ID failed");

    assert(pthread_mutex_lock(&lock) == 0 && "pthread_mutex_lock failed");
    printf("[%d~%d] consumed thread CPU time: %llu ns\n",
           i, cycles, (now - was));
    assert(pthread_mutex_unlock(&lock) == 0 && "pthread_mutex_unlock failed");
  }

  return NULL;
}

int
main(int argc, char *argv[])
{
  int c;
  ULL_t val;

  (void) argc; (void) argv;

    show_res(CLOCK_REALTIME);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_REALTIME (%d)      ", CLOCK_REALTIME);
      assert(get_time(CLOCK_REALTIME, &val) == 0 && "CLOCK_REALTIME failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_MONOTONIC);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC (%d)     ", CLOCK_MONOTONIC);
      assert(get_time(CLOCK_MONOTONIC, &val) == 0 && "CLOCK_MONOTONIC failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_MONOTONIC_RAW);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC_RAW (%d) ", CLOCK_MONOTONIC_RAW);
      assert(get_time(CLOCK_MONOTONIC_RAW, &val) == 0
             && "CLOCK_MONOTONIC_RAW failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_MONOTONIC_RAW_APPROX);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC_RAW_APPROX (%d) ", CLOCK_MONOTONIC_RAW_APPROX);
      assert(get_time(CLOCK_MONOTONIC_RAW_APPROX, &val) == 0
             && "CLOCK_MONOTONIC_RAW_APPROX failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_UPTIME_RAW);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_UPTIME_RAW (%d) ", CLOCK_UPTIME_RAW);
      assert(get_time(CLOCK_UPTIME_RAW, &val) == 0
             && "CLOCK_UPTIME_RAW failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_UPTIME_RAW_APPROX);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_UPTIME_RAW_APPROX (%d) ", CLOCK_UPTIME_RAW_APPROX);
      assert(get_time(CLOCK_UPTIME_RAW_APPROX, &val) == 0
             && "CLOCK_UPTIME_RAW_APPROX failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_PROCESS_CPUTIME_ID);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_PROCESS_CPUTIME_ID (%d) ", CLOCK_PROCESS_CPUTIME_ID);
      assert(get_time(CLOCK_PROCESS_CPUTIME_ID, &val) == 0
             && "CLOCK_PROCESS_CPUTIME_ID failed");
      printf("%llu\n", val);
    }

    show_res(CLOCK_THREAD_CPUTIME_ID);
    for (c = 0; ++c < 10;) {
      printf("CLOCK_THREAD_CPUTIME_ID (%d) ", CLOCK_THREAD_CPUTIME_ID);
      assert(get_time(CLOCK_THREAD_CPUTIME_ID, &val) == 0
             && "CLOCK_THREAD_CPUTIME_ID failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_REALTIME (%d) ", CLOCK_REALTIME);
      assert((val = clock_gettime_nsec_np(CLOCK_REALTIME)) != 0
             && "CLOCK_REALTIME failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC (%d) ", CLOCK_MONOTONIC);
      assert((val = clock_gettime_nsec_np(CLOCK_MONOTONIC)) != 0
             && "CLOCK_MONOTONIC failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC_RAW (%d) ", CLOCK_MONOTONIC_RAW);
      assert((val = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW)) != 0
             && "CLOCK_MONOTONIC_RAW failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_MONOTONIC_RAW_APPROX (%d) ", CLOCK_MONOTONIC_RAW_APPROX);
      assert((val = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX)) != 0
             && "CLOCK_MONOTONIC_RAW_APPROX failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_UPTIME_RAW (%d) ", CLOCK_UPTIME_RAW);
      assert((val = clock_gettime_nsec_np(CLOCK_UPTIME_RAW)) != 0
             && "CLOCK_UPTIME_RAW failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_UPTIME_RAW_APPROX (%d) ", CLOCK_UPTIME_RAW_APPROX);
      assert((val = clock_gettime_nsec_np(CLOCK_UPTIME_RAW_APPROX)) != 0
             && "CLOCK_UPTIME_RAW_APPROX failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_PROCESS_CPUTIME_ID (%d) ", CLOCK_PROCESS_CPUTIME_ID);
      assert((val = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID)) != 0
             && "CLOCK_PROCESS_CPUTIME_ID failed");
      printf("%llu\n", val);
    }

    printf("Testing clock_gettime_nsec_np()\n");
    for (c = 0; ++c < 10;) {
      printf("CLOCK_THREAD_CPUTIME_ID (%d) ", CLOCK_THREAD_CPUTIME_ID);
      assert((val = clock_gettime_nsec_np(CLOCK_THREAD_CPUTIME_ID)) != 0
             && "CLOCK_THREAD_CPUTIME_ID failed");
      printf("%llu\n", val);
    }

  {
    pthread_t t1, t2, t3, t4;
    int sleep1 = 100;
    int sleep2 = 100000;

    int cycles1 = 1000;
    int cycles2 = 10000000;

    assert(pthread_create(&t1, NULL, test_thread_sleep, &sleep1) == 0
           && "pthread_create failed");
    assert(pthread_create(&t2, NULL, test_thread_sleep, &sleep2) == 0
           && "pthread_create failed");
    assert(pthread_create(&t3, NULL, test_blackhole_thread, &cycles1) == 0
           && "pthread_create failed");
    assert(pthread_create(&t4, NULL, test_blackhole_thread, &cycles2) == 0
           && "pthread_create failed");

    assert(pthread_join(t1, NULL) == 0 && "pthread_join failed");
    assert(pthread_join(t2, NULL) == 0 && "pthread_join failed");
    assert(pthread_join(t3, NULL) == 0 && "pthread_join failed");
    assert(pthread_join(t4, NULL) == 0 && "pthread_join failed");
  }
  return 0;
}
