
#include <iostream>

#include <pthread.h>

#include <unistd.h>
#include <time.h>

static unsigned long long kSecondsToNanos = 1000ULL * 1000ULL * 1000ULL;

template< typename CLOCKID >
inline unsigned long long time( CLOCKID clk_id )
{
  struct timespec ts;

  std::cout << clock_gettime(clk_id, &ts) << " ";

  return (static_cast<unsigned long long>(ts.tv_sec) * kSecondsToNanos +
          static_cast<unsigned long long>(ts.tv_nsec));
}

template< typename CLOCKID >
inline void res( CLOCKID clk_id )
{
  struct timespec ts;
  std::cout << clock_getres(clk_id, &ts) << " ";
  std::cout << ts.tv_sec << " " << ts.tv_nsec
            << std::endl;
}

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void* test_thread_sleep(void *arg)
{
  int delay = *(int *)arg;
  int c = 0;

  while ( ++c < 10 )
  {
    unsigned long long was = time(CLOCK_THREAD_CPUTIME_ID);
    usleep(delay);
    unsigned long long now = time(CLOCK_THREAD_CPUTIME_ID);

    ::pthread_mutex_lock(&lock);
    std::cout << "[sleep for " << delay << "us] consumed thread CPU time: "<< now - was << " ns" << std::endl;
    ::pthread_mutex_unlock(&lock);
  }

  return NULL;
}

static void* test_blackhole_thread(void *arg)
{
  int cycles = *(int *)arg;
  int c = 0;
  while ( ++c < 10 )
  {
    unsigned long long was = time(CLOCK_THREAD_CPUTIME_ID);
    int i = 0;
    int cc = 0;
    while ( ++cc < cycles )
    {
      i = cc * cc % c;
    }
    unsigned long long now = time(CLOCK_THREAD_CPUTIME_ID);

    ::pthread_mutex_lock(&lock);
    std::cout << "[" << i << "~" << cycles << "] consumed thread CPU time: "<< now - was << " ns" << std::endl;
    ::pthread_mutex_unlock(&lock);
  }

  return NULL;
}

int main()
{
  {
    int c =0;
    res(CLOCK_REALTIME);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_REALTIME ("<< CLOCK_REALTIME << ")      " << time(CLOCK_REALTIME) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_MONOTONIC);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_MONOTONIC ("<< CLOCK_MONOTONIC << ")     " << time(CLOCK_MONOTONIC) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_MONOTONIC_RAW);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_MONOTONIC_RAW ("<< CLOCK_MONOTONIC_RAW << ") " << time(CLOCK_MONOTONIC_RAW) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_MONOTONIC_RAW_APPROX);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_MONOTONIC_RAW_APPROX ("<< CLOCK_MONOTONIC_RAW_APPROX << ") " << time(CLOCK_MONOTONIC_RAW_APPROX) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_UPTIME_RAW);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_UPTIME_RAW ("<< CLOCK_UPTIME_RAW << ") " << time(CLOCK_UPTIME_RAW) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_UPTIME_RAW_APPROX);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_UPTIME_RAW_APPROX ("<< CLOCK_UPTIME_RAW_APPROX << ") " << time(CLOCK_UPTIME_RAW_APPROX) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_PROCESS_CPUTIME_ID);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_PROCESS_CPUTIME_ID ("<< CLOCK_PROCESS_CPUTIME_ID << ") " << time(CLOCK_PROCESS_CPUTIME_ID) << std::endl;
    }
  }
  {
    int c = 0;
    res(CLOCK_THREAD_CPUTIME_ID);
    while ( ++c < 10 )
    {
      std::cout << "CLOCK_THREAD_CPUTIME_ID ("<< CLOCK_THREAD_CPUTIME_ID << ") " << time(CLOCK_THREAD_CPUTIME_ID) << std::endl;
    }
  }
  {
    pthread_t t1, t2, t3, t4;
    int sleep1 = 100;
    int sleep2 = 100000;

    int cycles1 = 1000;
    int cycles2 = 10000000;

    ::pthread_create(&t1, NULL, test_thread_sleep, &sleep1);
    ::pthread_create(&t2, NULL, test_thread_sleep, &sleep2);
    ::pthread_create(&t3, NULL, test_blackhole_thread, &cycles1);
    ::pthread_create(&t4, NULL, test_blackhole_thread, &cycles2);

    ::pthread_join(t1, NULL);
    ::pthread_join(t2, NULL);
    ::pthread_join(t3, NULL);
    ::pthread_join(t4, NULL);
  }
  return 0;
}
