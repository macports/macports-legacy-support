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
 * This provides some simple tests of the attrlist functions, both the
 * traditional set/get flavor, and the newer fget/fset flavor.  Tests of
 * "null" cases (no bits set) and "nobuf" cases (zero-length buffer) are
 * included, since those sometimes cause trouble.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/attr.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/vnode.h>

#ifdef __LP64__
typedef unsigned int attrlist_opts_t;
#else /* !__LP64__ */
typedef unsigned long attrlist_opts_t;
#endif /* !__LP64__ */

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define FINFO_TYPE    "TYPE"
#define FINFO_CREATOR "CRTR"
#define FINFO_TCSIZE  4

#define LL (long long)

typedef struct attrlist attrlist_t;
typedef struct timespec timespec_t;

/* FinderInfo alone */
typedef struct FInfoAttrBuf_s {
    uint8_t      finderInfo[32];
} FInfoAttrBuf_t;

/* Complete expected read buffer */
typedef struct rFInfoAttrBuf_s {
    u_int32_t       length;
    fsobj_type_t    objType;
    FInfoAttrBuf_t  FInfo;
} rFInfoAttrBuf_t;

static int
set_tc(FInfoAttrBuf_t *ab)
{
    if (strlen(FINFO_TYPE) != FINFO_TCSIZE
       || strlen(FINFO_CREATOR) != FINFO_TCSIZE) {
       printf("      *** type/creator lengths are not 4/4\n");
       return 1;
    }
    memset(ab, 0, sizeof(*ab));
    memcpy(&ab->finderInfo[0], FINFO_TYPE, FINFO_TCSIZE);
    memcpy(&ab->finderInfo[4], FINFO_CREATOR, FINFO_TCSIZE);
    return 0;
}

/*
 * There is a long-standing OS bug (10.5 and later) where attempting to set
 * the atime alone on an HFS+ (non-APFS?) file fails.  We include tests
 * for setting timestamps to check this.
 *
 * For completeness, we test all possible combinations of timestamp setting.
 * Also, since the "script" attribute comes earlier in the attribute sequence,
 * and hence affects the buffer positions of the timestamps, we optionally
 * "set" that as well, though without actually changing its value.  No other
 * settable attributes appear earlier in the buffer than the timestamps.
 *
 * For each timestamp being set, we attempt to set it two seconds earlier
 * than its current value.  We leave the nanoseconds alone, thereby avoiding
 * issues related to whether the filesystem supports sub-second timestamps.
 * Since there are a total of 31 cases, and each timestamp is changed in half
 * of the cases, if everything worked correctly we'd see each timestamp
 * moved backwards by 32 seconds.
 */

/* Attributes to test (in order of appearance) */
#define ATTR_TESTS (ATTR_CMN_SCRIPT \
                    | ATTR_CMN_CRTIME | ATTR_CMN_MODTIME \
                    | ATTR_CMN_CHGTIME | ATTR_CMN_ACCTIME)

/* Least-significant test bit (for incrementing) */
#define ATTR_TESTS_LSB (ATTR_TESTS & (~ATTR_TESTS + 1))

/* Test mask should be contiguous */
#if (ATTR_TESTS + ATTR_TESTS_LSB) & ATTR_TESTS
#error ATTR_TESTS is not contiguous
#endif

/* Structure for all timestamp-test attributes, including the (read) count */
/* The initial two 32-bit items cause the timespecs to be naturally aligned. */
typedef struct tsattr_s {
    u_int32_t       length;
    text_encoding_t script;
    timespec_t      crtime;
    timespec_t      modtime;
    timespec_t      chgtime;
    timespec_t      acctime;
} tsattr_t;

const char * const getname[] = {
    "getattrlist", "fgetattrlist", "getattrlistat",
    };
const char * const setname[] = {
    "setattrlist", "fsetattrlist", "setattrlistat",
    };

static int
xgetattrlist(int mode, const char* path, int fd, attrlist_t *attrList,
             void *attrBuf, size_t attrBufSize, attrlist_opts_t options)
{
  switch (mode) {

  case 0:
    return getattrlist(path, attrList, attrBuf, attrBufSize, options);

  case 1:
    return fgetattrlist(fd, attrList, attrBuf, attrBufSize, options);

  case 2:
    return getattrlistat(fd, path, attrList, attrBuf, attrBufSize, options);
  }
  return 0;  /* Shouldn't get here */
}

static int
xsetattrlist(int mode, const char* path, int fd, attrlist_t *attrList,
             void *attrBuf, size_t attrBufSize, attrlist_opts_t options)
{
  switch (mode) {

  case 0:
    return setattrlist(path, attrList, attrBuf, attrBufSize, options);

  case 1:
    return fsetattrlist(fd, attrList, attrBuf, attrBufSize, options);

  case 2:
    return setattrlistat(fd, path, attrList, attrBuf, attrBufSize, options);
  }
  return 0;  /* Shouldn't get here */
}

static int
get_tsattrs(int mode, const char* path, int fd, tsattr_t *tsb)
{
  int ret;
  attrlist_t al = {.bitmapcount = ATTR_BIT_MAP_COUNT,
                   .commonattr = ATTR_TESTS};

  ret = xgetattrlist(mode, path, fd, &al, tsb, sizeof(*tsb), 0);
  if (ret) {
    printf("      *** %s() for timestamps failed: %s\n",
           getname[mode], strerror(errno));
    return ret;
  }
  if (tsb->length != sizeof(*tsb)) {
    printf("      *** %s() for timestamps returned"
           " length %d, should be %d\n",
           getname[mode], tsb->length, (int) sizeof(*tsb));
  }
  return 0;
}

static int
set_tsattrs(int mode, const char* path, int fd, tsattr_t *tsb,
            attrgroup_t attrs)
{
  int ret;
  attrlist_t al = {.bitmapcount = ATTR_BIT_MAP_COUNT,
                   .commonattr = attrs};
  uint8_t wbuf[sizeof(*tsb)], *wbp = &wbuf[0];

  if (attrs & ATTR_CMN_SCRIPT) {
    *((text_encoding_t *) wbp) = tsb->script;
    wbp += sizeof(text_encoding_t);
  }
  if (attrs & ATTR_CMN_CRTIME) {
    tsb->crtime.tv_sec -= 2;
    *((timespec_t *) wbp) = tsb->crtime;
    wbp += sizeof(timespec_t);
  }
  if (attrs & ATTR_CMN_MODTIME) {
    tsb->modtime.tv_sec -= 2;
    *((timespec_t *) wbp) = tsb->modtime;
    wbp += sizeof(timespec_t);
  }
  if (attrs & ATTR_CMN_CHGTIME) {
    tsb->chgtime.tv_sec -= 2;
    *((timespec_t *) wbp) = tsb->chgtime;
    wbp += sizeof(timespec_t);
  }
  if (attrs & ATTR_CMN_ACCTIME) {
    tsb->acctime.tv_sec -= 2;
    *((timespec_t *) wbp) = tsb->acctime;
    wbp += sizeof(timespec_t);
  }

  ret = xsetattrlist(mode, path, fd, &al, &wbuf, wbp - &wbuf[0], 0);
  if (ret) {
    printf("      *** %s() for timestamps failed: %s\n",
           setname[mode], strerror(errno));
    return ret;
  }
  return 0;
}

static int
check_ts(timespec_t *out, timespec_t *in,
         const char *name, attrgroup_t bit, attrgroup_t mask)
{
  if (in->tv_sec != out->tv_sec || in->tv_nsec != out->tv_nsec) {
    printf("      *** %s (%sset) mismatches,"
           " sent = %lld.%09lld, rcvd = %lld.%09lld, mask = %04X\n",
           name, mask & bit ? "" : "not ",
           LL out->tv_sec, LL out->tv_nsec, LL in->tv_sec, LL in->tv_nsec,
           mask);
    return 1;
  }
  return 0;
}

static int
check_tsattrs(tsattr_t *out, tsattr_t *in, attrgroup_t attrs,
              int verbose, int testmode)
{
  int ret = 0;

  if (in->script != out->script) {
    printf("      *** SCRIPT (%sset) mismatches,"
           " sent = %u, rcvd = %u, mask = %04X\n",
           attrs &ATTR_CMN_SCRIPT ? "" : "un",
           out->script, in->script, attrs);
    ret = 1;
  }

  ret |= check_ts(&out->crtime, &in->crtime,
                  "CRTIME", ATTR_CMN_CRTIME, attrs);
  ret |= check_ts(&out->modtime, &in->modtime,
                  "MODTIME", ATTR_CMN_MODTIME, attrs);
  if (testmode > 2) {  /* Setting CHGTIME is unsupported, even for root */
    ret |= check_ts(&out->chgtime, &in->chgtime,
                    "CHGTIME", ATTR_CMN_CHGTIME, attrs);
  }
  ret |= check_ts(&out->acctime, &in->acctime,
                  "ACCTIME", ATTR_CMN_ACCTIME, attrs);

  return ret;
}

static int
do_tstests(int mode, const char *path, int fd, int apfs,
           int verbose, int testmode)
{
  int ret = 0, err;
  attrgroup_t mask;
  tsattr_t tsb1, tsb2;

  if (verbose) printf("    testing timestamps\n");
  if ((err = get_tsattrs(mode, path, fd, &tsb1))) return err;

  for (mask = ATTR_TESTS_LSB; mask & ATTR_TESTS; mask += ATTR_TESTS_LSB) {
    if (verbose > 1) printf("      testing case %04X\n", mask);
    if ((err = set_tsattrs(mode, path, fd, &tsb1, mask))) return err;
    if ((err = get_tsattrs(mode, path, fd, &tsb2))) return err;
    if ((err = check_tsattrs(&tsb1, &tsb2, mask, verbose, testmode))) {
      /* Setting atime alone on non-APFS doesn't work */
      /* On 10.11+, it also fails if attempting to set ctime as well */
      if (!apfs && testmode < 2
          && (mask == ATTR_CMN_ACCTIME
              || mask == (ATTR_CMN_ACCTIME | ATTR_CMN_CHGTIME))) {
        printf("        *** tolerating known OS bug\n");
      } else {
        ret |= err;
      }
    }
    tsb1 = tsb2;
  }

  return ret;
}

static int
do_tests(int mode, const char *path, const char *rpath, int apfs,
         int verbose, int testmode)
{
  int ret = 0, dirfd = -1, fd = -1, xfd, err;
  const char *xpath;
  attrlist_t al = {.bitmapcount = ATTR_BIT_MAP_COUNT};
  rFInfoAttrBuf_t abr; FInfoAttrBuf_t abw;

  const char *get = getname[mode];
  const char *set = setname[mode];

  do {
    if (verbose) printf("  opening '" TEST_TEMP "'\n");
    if ((dirfd = open(TEST_TEMP, O_RDONLY)) < 0) {
       printf("    *** unable to open '" TEST_TEMP "': %s\n", strerror(errno));
       ret = 1;
       break;
    }

    if (verbose) printf("  creating '%s'\n", path);
    if ((fd = open(path, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
       printf("    *** unable to open '%s': %s\n", path, strerror(errno));
       ret = 1;
       break;
    }

    if (mode == 2) {
      xfd = dirfd;
      xpath = rpath;
    } else {
      xfd = fd;
      xpath = path;
    }

    if (verbose) printf("    testing '%s'\n", get);
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    if (xgetattrlist(mode, xpath, xfd, &al, &abr, sizeof(abr), 0)) {
      printf("      *** %s() for '%s' failed: %s\n",
             get, xpath, strerror(errno));
      ret = 1;
    } else {
      if (abr.length != sizeof(abr)) {
        printf("      *** %s() returned length %d, should be %d\n",
               get, abr.length, (int) sizeof(abr));
        ret = 1;
      }
      if (abr.objType != VREG) {
        printf("      *** '%s' is not a regular file.\n", path);
        ret = 1;
      }

      if (verbose) printf("    testing '%s'\n", set);
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      if (xsetattrlist(mode, xpath, xfd, &al, &abw, sizeof(abw), 0)) {
        printf("      *** %s() for '%s' failed: %s\n",
               set, xpath, strerror(errno));
        ret = 1;
      } 

      if (verbose) printf("    testing nobuf '%s'\n", set);
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      err = xsetattrlist(mode, xpath, xfd, &al, &abw, 0, 0);
      if (err) {
        if (errno == ENOMEM || errno == EINVAL) {
          if (verbose) printf("      failed as expected: %s\n",
                              strerror(errno));
        } else {
          printf("      *** nobuf %s() for '%s' failed: %s\n",
                 set, xpath, strerror(errno));
          ret = 1;
        }
      } else {
        printf("      *** nobuf %s() unexpectedly succeeded\n", set);
        ret = 1;
      }
    }

    if (verbose) printf("    testing null '%s'\n", get);
    al.commonattr = 0;
    abr.length = -1;
    if (xgetattrlist(mode, xpath, xfd, &al, &abr, sizeof(abr), 0)) {
      printf("      *** null %s() for '%s' failed: %s\n",
             get, xpath, strerror(errno));
      ret = 1;
    } else {
      /* Null result should have length of just the length field alone. */
      if (abr.length != sizeof(abr.length)) {
        printf("      *** null %s() returned length %d, should be %d\n",
               get, abr.length, (int) sizeof(abr.length));
        ret = 1;
      }
    }

    if (verbose) printf("    testing nobuf '%s'\n", get);
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    /* Silent truncation means "success" */
    err = xgetattrlist(mode, xpath, xfd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("      failed as expected: %s\n", strerror(errno));
      } else {
        printf("      *** nobuf %s() for '%s' failed: %s\n",
               get, xpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("    testing null/nobuf '%s'\n", get);
    al.commonattr = 0;
    abr.length = -1;
    err = xgetattrlist(mode, xpath, xfd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("      failed as expected: %s\n", strerror(errno));
      } else {
        printf("      *** null/nobuf %s() for '%s' failed: %s\n",
               get, xpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("    testing null '%s'\n", set);
    al.commonattr = 0;
    if (xsetattrlist(mode, xpath, xfd, &al, &abw, sizeof(abw), 0)) {
      printf("      *** null %s() for '%s' failed: %s\n",
             set, xpath, strerror(errno));
      ret = 1;
    }

    if (verbose) printf("    testing null/nobuf '%s'\n", set);
    al.commonattr = 0;
    if (xsetattrlist(mode, xpath, xfd, &al, &abw, 0, 0)) {
      printf("      *** null/nobuf %s() for '%s' failed: %s\n",
             set, xpath, strerror(errno));
      ret = 1;
    }

    if (testmode) ret |= do_tstests(mode, xpath, xfd, apfs, verbose, testmode);
  } while (0);

  if (fd >= 0) (void) close(fd);

	if (verbose) printf("  deleting '%s'\n", path);
	if (unlink(path)) {
		printf("    *** error deleting '%s': %s (%d)\n",
					 path, strerror(errno), errno);
	}

  if (dirfd >=0) {
    if (verbose) printf("  closing '" TEST_TEMP "'\n");
    (void) close(dirfd);
  }

  return ret;
}

int
main(int argc, char *argv[])
{
  int argn = 1, verbose = 0, testmode = 0, ret = 0, apfs;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  const char *cp;
  char chr;
  char tpath[MAXPATHLEN], rpath[MAXPATHLEN];
  struct statfs sfs = {0};

  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 't': ++testmode; break;
        case 'v': ++verbose; break;
      }
    }
    ++argn;
  }

  (void) snprintf(tpath, sizeof(tpath), TEST_TEMP "/%s-%u", progname, pid);
  (void) snprintf(rpath, sizeof(rpath), "%s-%u", progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  if (statfs(TEST_TEMP, &sfs)) {
    printf("  *** statfs() for '" TEST_TEMP "' failed: %s (%d)\n",
           strerror(errno), errno);
    ret = 1;
  } else {
    if (verbose) printf("  filesystem type is '%s'\n", sfs.f_fstypename);
    apfs = !strcmp(sfs.f_fstypename, "apfs");

    ret = do_tests(0, tpath, rpath, apfs, verbose, testmode);
    ret |= do_tests(1, tpath, rpath, apfs, verbose, testmode);
    ret |= do_tests(2, tpath, rpath, apfs, verbose, testmode);
  }

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
