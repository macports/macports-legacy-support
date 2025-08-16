/*
 * Simple test harness and benchmark for MT Arc4Random
 *
 * Note that this is almost entirely just a benchmark, and not an actual
 * test of the randomness of the result, which would be complicated.  The
 * only thing really being "tested" is that the function can be called.
 */
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Performance counter access.
 *
 * NB: Relative cycle counts and difference between two
 *     cpu-timestamps are meaningful ONLY when run on the _same_ CPU.
 */
#if defined(__i386__)

static inline uint64_t sys_cpu_timestamp(void)
{
    uint64_t x;
    __asm__ volatile ("rdtsc" : "=A" (x));
    return x;
}

#elif defined(__x86_64__)

static inline uint64_t sys_cpu_timestamp(void)
{
    uint64_t res;
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));

    res = hi;
    return (res << 32) | lo;
}

#else

#include <mach/mach.h>
#include <mach/mach_time.h>
static inline uint64_t sys_cpu_timestamp(void)
{
    return(mach_absolute_time());
}

#endif /* x86, x86_64 */

/*
 * Generate 'siz' byte RNG in a tight loop and provide averages.
 */
static void
bench(int fd, size_t siz, size_t niter, int verbose)
{
    size_t j;
    uint8_t *buf = malloc(siz);
    uint64_t s0, s1, s2;
    uint64_t ta = 0;    // cumulative time for arc4rand
    uint64_t ts = 0;    // cumulative time for system rand
    ssize_t m;

    for (j = 0; j < niter; ++j) {
        s0 = sys_cpu_timestamp();
        arc4random_buf(buf, siz);
        s1 = sys_cpu_timestamp();
        m  =  read(fd, buf, siz); (void) m;
        s2 = sys_cpu_timestamp();

        ta += s1 - s0;
        ts += s2 - s1;
    }

#define _d(x)   ((double)(x))
    double aa = _d(ta) / _d(niter);     // average of n runs for arc4random
    double as = _d(ts) / _d(niter);     // average of n runs for sysrand
    double sa = aa / _d(siz);           // cycles/byte for arc4random
    double ss = as / _d(siz);           // cycles/byte for sysrand

    double speedup = ss / sa;

    if (verbose) printf("%6lu,\t%9.4f,\t%9.4f,\t%6.2f\n", siz, sa, ss, speedup);

    free(buf);
}

#define NITER       8192

int
main(int argc, char *argv[])
{
  int verbose = 0;
  char *progname = basename(argv[0]);
  int i;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s started\n", progname);

  int fd = open("/dev/urandom", O_RDONLY);

  int args[]  = { 16, 32, 64, 256, 512 };
  int nargs = 5;

  if (verbose) printf("  size,\t arc4rand,\t  sysrand,\tspeedup\n");
  for (i = 0; i < nargs; ++i) {
    int z = args[i];
    if (z <= 0) continue;
    bench(fd, z, NITER, verbose);
  }

  close(fd);

  printf("%s completed.\n", progname);
  return 0;
}
