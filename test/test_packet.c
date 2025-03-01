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
 * Test for packet-related issues, currently just timestamps.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <netinet/in.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>

#include <mach/mach_time.h>

#define SYSCTL_OSVER_CLASS CTL_KERN
#define SYSCTL_OSVER_ITEM  KERN_OSRELEASE

#define CMSG_DATALEN(cmsg) ((uint8_t *) (cmsg) + (cmsg)->cmsg_len \
	                    - (uint8_t *) CMSG_DATA(cmsg))

typedef struct timeval timeval_t;

#define BILLION64 1000000000ULL
#define MAX_TV_USEC 1000000
#define MAX_TV_NSEC 1000000000
#define MAX_MACH_TIME (1ULL << 60) /* Arbitrary >36-year uptime limit */

#define TS_TYPES \
  TS_ONE(none,,0,NULL,0) \
  TS_ONE(tv,struct timeval,sizeof(timeval_t),get_timeval_ts,0) \
  TS_ONE(u64mach,uint64_t (mach),sizeof(uint64_t),get_mach_ts,11) \

#define TS_ONE(name,str,size,get,minver) ts_##name,
typedef enum ts_type {
  TS_TYPES
} ts_type_t;
#undef TS_ONE

#define TS_ONE(name,str,size,get,minver) #str,
static const char * const ts_type_names[] = {
  TS_TYPES
};
#undef TS_ONE

#define TS_ONE(name,str,size,get,minver) size,
static const int ts_sizes[] = {
  TS_TYPES
};
#undef TS_ONE

#define TS_ONE(name,str,size,get,minver) minver,
static const int ts_min_darwin[] = {
  TS_TYPES
};
#undef TS_ONE

/*
 * Handling for mismatched timestamp formats, copied from ntpsec
 *
 * Since this particular bug is not currently fixed here, we duplicate
 * the ntpsec workaround to avoid a test failure.  This is just a verbatim
 * copy, and handles more issues than we'll see here.
 */

static const union {
	uint8_t c[8];
	uint64_t i;
} endian_test = {{1, 2, 3, 4, 5, 6, 7, 8}};
#define ENDIAN_LITTLE 0x0807060504030201ULL
#define IS_LITTLE_ENDIAN (endian_test.i == ENDIAN_LITTLE)

static struct timeval *
access_cmsg_timeval(struct cmsghdr *cmsghdr, struct timeval *temp)
{
	struct timeval_3232 {
		uint32_t	tv_sec;  /* Unsigned to get past 2038 */
		int32_t		tv_usec;
	} *tv3232p;
	struct timeval_6432 {
		int64_t		tv_sec;
		int32_t		tv_usec;
	} *tv6432p;
	#define SIZEOF_PACKED_TIMEVAL6432 (sizeof(int64_t) + sizeof(int32_t))
	struct timeval_6464 {
		int64_t		tv_sec;
		uint32_t	tv_usec[2];  /* Unsigned for compares */
	} *tv6464p;
	int datalen = CMSG_DATALEN(cmsghdr);

	(void) access_cmsg_timeval;

	if (datalen == sizeof(struct timeval)) {
		return (struct timeval *) CMSG_DATA(cmsghdr);
	}

	switch (datalen) {

	case sizeof(struct timeval_3232):
		tv3232p = (struct timeval_3232 *) CMSG_DATA(cmsghdr);
		temp->tv_sec = tv3232p->tv_sec;
		temp->tv_usec = tv3232p->tv_usec;
		break;

	case SIZEOF_PACKED_TIMEVAL6432:
		tv6432p = (struct timeval_6432 *) CMSG_DATA(cmsghdr);
		temp->tv_sec = tv6432p->tv_sec;
		temp->tv_usec = tv6432p->tv_usec;
		break;

	case sizeof(struct timeval_6464):
		tv6464p = (struct timeval_6464 *) CMSG_DATA(cmsghdr);
		temp->tv_sec = tv6464p->tv_sec;
		if (IS_LITTLE_ENDIAN) {
			temp->tv_usec = tv6464p->tv_usec[0];
			break;
		} else if (tv6464p->tv_usec[0] == 0) {
			if (tv6464p->tv_usec[1] < MAX_TV_USEC) {
				temp->tv_usec = tv6464p->tv_usec[1];
			} else {
				temp->tv_usec = tv6464p->tv_usec[0];
			}
			break;
		} else if (tv6464p->tv_usec[0] < MAX_TV_USEC) {
			temp->tv_usec = tv6464p->tv_usec[0];
			break;
		}
		/* FALLTHRU to default (invalid timestamp) */

	default:
		memset(temp, 0, sizeof(struct timeval));
	}

	return temp;
}

/* Timestamp checks & conversions to uint64 */

static uint64_t
timeval2nanos(timeval_t *tvp)
{
  return tvp->tv_sec * BILLION64 + tvp->tv_usec * 1000;
}

typedef uint64_t ts_func_t(struct cmsghdr *cmsghdr);

static uint64_t
get_timeval_ts(struct cmsghdr *cmsghdr)
{
  timeval_t tvtemp;
  timeval_t *tvp = access_cmsg_timeval(cmsghdr, &tvtemp);

  if ((uint64_t) tvp->tv_usec >= MAX_TV_USEC) return 0;
  return timeval2nanos(tvp);
}

static uint64_t
get_mach_ts(struct cmsghdr *cmsghdr)
{
  uint64_t val = *((uint64_t *) CMSG_DATA(cmsghdr));

  return val >= MAX_MACH_TIME ? 0 : val;
}

#define TS_ONE(name,str,size,get,minver) get,
static ts_func_t *ts_getters[] = {
  TS_TYPES
};
#undef TS_ONE

static unsigned long long
mach2ns(uint64_t mach_time)
{
  static mach_timebase_info_data_t mach_scale = {0};

  if (!mach_scale.numer || !mach_scale.denom) {
    if (mach_timebase_info(&mach_scale) != KERN_SUCCESS) return mach_time;
  }
  return (double) mach_time * mach_scale.numer / mach_scale.denom;
}

/* Data for tests */

static const char sample[] = "The Quick Brown Fox";

static uint8_t dbuf[256];
static uint8_t cbuf[256];
static char osver[256];
static uint32_t darwinver = 0;

static int
get_osver(void)
{
  int mib[] = {SYSCTL_OSVER_CLASS, SYSCTL_OSVER_ITEM};
  int miblen = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(osver);
  char *cp;

  if (sysctl(mib, miblen, osver, &len, NULL, 0)) return -1;
  if (len <= 0 || len >= (ssize_t) sizeof(osver)) return -1;
  osver[len] = '\0';
  if (osver[len - 1] == '\n') osver[len - 1] = '\0';

  darwinver = strtoul(osver, &cp, 10);
  if (!darwinver || (*cp && *cp != '.')) return 1;

  return 0;
}

typedef struct times_s {
  timeval_t tv1;
  uint64_t mt1;
  timeval_t tv2;
  uint64_t mt2;
} times_t;

/* Test one packet via loopback */

static const char *
test_packet(int sockopt, socklen_t *cbuflen, times_t *tp, ts_type_t tstype)
{
  const char *err = NULL;
  int saverr, sockin = -1, sockout = -1;
  struct sockaddr_in addrin = {.sin_family = AF_INET};
  struct sockaddr_in addrout = {.sin_family = AF_INET};
  struct sockaddr_in addrinused, addroutused;
  socklen_t addrinused_len, addroutused_len;
  struct iovec msgiov = {.iov_base = dbuf, .iov_len = sizeof(dbuf)};
  struct msghdr hdr = {
      .msg_name = NULL,
      .msg_namelen = 0,
      .msg_iov = &msgiov,
      .msg_iovlen = 1,
      };
  static const int trueval = 1;
  uint32_t *cbufp;

  hdr.msg_control = cbuf;
  hdr.msg_controllen = *cbuflen;

  /* Make unpopulated buffer obvious */
  cbufp = (uint32_t *) cbuf;
  while (cbufp < (uint32_t *) (cbuf + sizeof(cbuf))) {
    *cbufp++ = 0xDEADBEEFU;
  }

  do {
    if ((sockin = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      err = "socket() in";
      break;
    }
    if ((sockout = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      err = "socket() out";
      break;
    }

    if (sockopt && setsockopt(sockin, SOL_SOCKET, sockopt,
                              (const void *) &trueval, sizeof(trueval)) < 0) {
      if (errno == ENOPROTOOPT && darwinver < ts_min_darwin[tstype]) {
        err = "*setsockopt()";
      } else {
        err = "setsockopt()";
      }
      break;
    }

    addrin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sockin, (struct sockaddr *) &addrin, sizeof(addrin))) {
      err = "bind() in";
      break;
    }
    addrinused_len = sizeof(addrinused);
    if (getsockname(sockin,
                    (struct sockaddr *) &addrinused, &addrinused_len)) {
      err = "getsockname() in";
      break;
    }

    addrout.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sockout, (struct sockaddr *) &addrout, sizeof(addrout))) {
      err = "bind() out";
      break;
    }
    addroutused_len = sizeof(addroutused);
    if (getsockname(sockout,
                    (struct sockaddr *) &addroutused, &addroutused_len)) {
      err = "getsockname() out";
      break;
    }

    if (connect(sockin, (struct sockaddr *) &addroutused, addroutused_len)) {
      err = "connect() in->out";
      break;
    }
    if (connect(sockout, (struct sockaddr *) &addrinused, addrinused_len)) {
      err = "connect() out->in";
      break;
    }

    if (gettimeofday(&tp->tv1, NULL)) {
      err = "pre-send gettimeofday()";
      break;
    }
    switch (tstype) {

    case ts_u64mach:
      if (!(tp->mt1 = mach_absolute_time())) {
        err = "pre-send mach_absolute_time()";
      }
      break;

    default: break;
    }
    if (err) break;
    if (send(sockout, sample, sizeof(sample), 0) != sizeof(sample)) {
      err = "send()";
      break;
    }
    if (recvmsg(sockin, &hdr, 0) != sizeof(sample)) {
      err = "recvmsg()";
      break;
    }
    switch (tstype) {

    case ts_u64mach:
      if (!(tp->mt2 = mach_absolute_time())) {
        err = "post-recv mach_absolute_time()";
      }
      break;

    default: break;
    }
    if (err) break;
    if (gettimeofday(&tp->tv2, NULL)) {
      err = "post-recv gettimeofday()";
      break;
    }
    *cbuflen = hdr.msg_controllen;

    if (strncmp((const char *) dbuf, sample, sizeof(sample))) {
      err = "received data compare";
    }
  } while(0);

  saverr = errno;
  if (sockout >= 0) (void) close(sockout);
  if (sockin >= 0) (void) close(sockin);
  errno = saverr;
  return err;
}

static int
test_timestamp(const char *name, ts_type_t tstype, int sockopt, int scmtype,
               int verbose)
{
  int ret = 0;
  const char *tsname = ts_type_names[tstype];
  const char *err = NULL;
  times_t times;
  unsigned long long time1n, time2n, tsval, tsvalns = 0, tslow, tshigh;
  socklen_t cbuflen, cmsglen;
  struct cmsghdr *cmsg;
  int cmsglvl, cmsgtype, hdrlen, datalen, xdatalen;
  uint32_t *datap, *xdatap;

  if (verbose) printf("  Testing %s:\n", name);
  cbuflen = sizeof(cbuf);
  err = test_packet(sockopt, &cbuflen, &times, tstype);
  if (err) {
    if (*err == '*') {
      printf("    Allowable error on %s: %s\n", ++err, strerror(errno));
      return 0;
    } else {
      printf("    Error on %s: %s\n", err, strerror(errno));
      return 1;
    }
  }

  time1n = timeval2nanos(&times.tv1);
  time2n = timeval2nanos(&times.tv2);
  if (cbuflen < sizeof(struct cmsghdr)) {
    cmsglen = 0;
    cmsglvl = 0;
    cmsgtype = -1;
    cmsg = NULL;
  } else {
    cmsg = (struct cmsghdr *) cbuf;
    cmsglen = cmsg->cmsg_len;
    cmsglvl = cmsg->cmsg_level;
    cmsgtype = cmsg->cmsg_type;
  }
  if (verbose) {
    printf("    Sent at %llu.%09llu, received at %llu.%09llu, took %llu ns\n",
           time1n / BILLION64, time1n % BILLION64,
           time2n / BILLION64, time2n % BILLION64,
           time2n - time1n);
    switch (tstype) {

    case ts_u64mach:
      printf("    Mach times (ns) %llu, %llu, diff = %llu\n",
             mach2ns(times.mt1), mach2ns(times.mt2),
             mach2ns(times.mt2 - times.mt1));
      break;

    default: break;
    }
  }
  if (cmsgtype != scmtype) {
    printf("    Unexpected cmsg_type %d != %d\n", cmsgtype, scmtype);
    return 1;
  }
  if (!cmsglen) return 0;
  if (cmsg->cmsg_level != SOL_SOCKET) {
    printf("    Unexpected cmsg_level %d != SOL_SOCKET\n", cmsg->cmsg_level);
    return 1;
  }

  datalen = CMSG_DATALEN(cmsg);
  hdrlen = cmsglen - datalen;
  datap = (uint32_t *) CMSG_DATA(cmsg);
  if (verbose) printf("    cmsg length = %d (%d+%d), level = %d, type = %d\n",
                      (int) cmsglen, hdrlen, datalen, cmsglvl, cmsgtype);
  if (ts_sizes[tstype] && datalen != ts_sizes[tstype]) {
    printf("    %s payload length %d != expected sizeof(%s) = %d\n",
           name, datalen, tsname, ts_sizes[tstype]);
  }
  if (hdrlen > (int) sizeof(*cmsg)) {
    xdatap = (uint32_t *) (cbuf + sizeof(*cmsg));
    printf("    %s header padding:\n", name);
    while (xdatap < datap) {
      printf("     (%10u)\n", *xdatap++);
    }
    ret = 1;
  }
  if (verbose) {
    printf("    %s%spayload longwords:\n", tsname, tsname[0] ? " " : "");
    while (datap < (uint32_t *) (cbuf + cmsglen)) {
      printf("      %10u  (0x%08X)\n", *datap, *datap);
      ++datap;
    }
    /* Check for extra data assumed by simple pointer cast */
    xdatalen = ts_sizes[tstype] - datalen;
    if (xdatalen > 0) {
      datap = (uint32_t *) ((uint8_t *) CMSG_DATA(cmsg) + datalen);
      xdatap = datap + xdatalen / sizeof(uint32_t);
      printf("    erroneously assumed additional payload longwords:\n");
      while (datap < xdatap) {
        printf("     (%10u)  (0x%08X)\n", *datap, *datap);
        ++datap;
      }
    } else xdatalen = 0;
    if (cbuflen > cmsglen + xdatalen) {
      printf("    +%d bytes of additional cmsg data\n",
             (int) (cbuflen - cmsglen));
    }
  }

  if (ts_getters[tstype]) {
    tsval = (*ts_getters[tstype])(cmsg);
    if (!tsval) {
      printf("    %s provided invalid %s\n", name, tsname);
      ret = 1;
    } else {
      switch (tstype) {

      case ts_tv:
        tslow = time1n; tshigh = time2n; tsvalns = tsval;
        break;

      case ts_u64mach:
        tslow = times.mt1; tshigh = times.mt2; tsvalns = mach2ns(tsval);
        break;

      default:
        tslow = 0; tshigh = ~0ULL; tsvalns = tsval;
        break;
      }
      if (tsval < tslow || tsval > tshigh) {
        printf("    %s %s value %llu is not between %llu and %llu\n",
               name, tsname, tsval, tslow, tshigh);
        ret = 1;
      }
    }
    if (verbose && !ret) printf("    %s value (ns) is %llu\n",
                                tsname, tsvalns);
  }

  return ret;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, err = 0;
  char *name = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) printf("%s starting.\n", name);

  if (get_osver()) {
    printf("Can't get OS version\n");
    err = 1;
  } else if (verbose) printf("OS is Darwin %s\n", osver);

  err |= test_timestamp("(no timestamp)", ts_none, 0, -1, verbose);
  err |=test_timestamp("SO_TIMESTAMP", ts_tv, SO_TIMESTAMP, SCM_TIMESTAMP,
                       verbose);
  /* macOS enhancement in 10.7+ */
  #ifdef SO_TIMESTAMP_MONOTONIC
  err |= test_timestamp("SO_TIMESTAMP_MONOTONIC", ts_u64mach,
                        SO_TIMESTAMP_MONOTONIC, SCM_TIMESTAMP_MONOTONIC,
                        verbose);
  #endif
  /* The following is in macOS 10.14+ kernel sources, but not user headers. */
  #ifdef SO_TIMESTAMP_CONTINUOUS
  err |= test_timestamp("SO_TIMESTAMP_CONTINUOUS", ts_u64mach,
                        SO_TIMESTAMP_CONTINUOUS, SCM_TIMESTAMP_CONTINUOUS,
                        verbose);
  #endif

  printf("%s %s.\n", name, err ? "failed" : "passed");
  return err;
}
