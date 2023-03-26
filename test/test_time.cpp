
#include <iostream>
//#include <cstdint>

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
  return 0;
}
