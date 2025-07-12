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

typedef struct attrlist attrlist_t;

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

static int
xgetattrlist(int mode, const char* path, int fd, attrlist_t *attrList,
             void *attrBuf, size_t attrBufSize, attrlist_opts_t options)
{
  return mode ? fgetattrlist(fd, attrList, attrBuf, attrBufSize, options)
              : getattrlist(path, attrList, attrBuf, attrBufSize, options);
}

static int
xsetattrlist(int mode, const char* path, int fd, attrlist_t *attrList,
             void *attrBuf, size_t attrBufSize, attrlist_opts_t options)
{
  return mode ? fsetattrlist(fd, attrList, attrBuf, attrBufSize, options)
              : setattrlist(path, attrList, attrBuf, attrBufSize, options);
}

static int
do_tests(int mode, const char *path, int verbose)
{
  int ret = 0, fd = -1, err;
  attrlist_t al = {.bitmapcount = ATTR_BIT_MAP_COUNT};
  rFInfoAttrBuf_t abr; FInfoAttrBuf_t abw;

  const char *get = mode ? "fgetattrlist" : "getattrlist";
  const char *set = mode ? "fsetattrlist" : "setattrlist";

  do {
    if (verbose) printf("  creating '%s'\n", path);
    if ((fd = open(path, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
       printf("    *** unable to open '%s': %s\n", path, strerror(errno));
       ret = 1;
       break;
    }

    if (verbose) printf("    testing '%s'\n", get);
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    if (xgetattrlist(mode, path, fd, &al, &abr, sizeof(abr), 0)) {
      printf("      *** %s() for '%s' failed: %s\n",
             get, path, strerror(errno));
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
      if (xsetattrlist(mode, path, fd, &al, &abw, sizeof(abw), 0)) {
        printf("      *** %s() for '%s' failed: %s\n",
               set, path, strerror(errno));
        ret = 1;
      } 

      if (verbose) printf("    testing nobuf '%s'\n", set);
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      err = xsetattrlist(mode, path, fd, &al, &abw, 0, 0);
      if (err) {
        if (errno == ENOMEM || errno == EINVAL) {
          if (verbose) printf("      failed as expected: %s\n",
                              strerror(errno));
        } else {
          printf("      *** nobuf %s() for '%s' failed: %s\n",
                 set, path, strerror(errno));
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
    if (xgetattrlist(mode, path, fd, &al, &abr, sizeof(abr), 0)) {
      printf("      *** null %s() for '%s' failed: %s\n",
             get, path, strerror(errno));
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
    err = xgetattrlist(mode, path, fd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("      failed as expected: %s\n", strerror(errno));
      } else {
        printf("      *** nobuf %s() for '%s' failed: %s\n",
               get, path, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("    testing null/nobuf '%s'\n", get);
    al.commonattr = 0;
    abr.length = -1;
    err = xgetattrlist(mode, path, fd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("      failed as expected: %s\n", strerror(errno));
      } else {
        printf("      *** null/nobuf %s() for '%s' failed: %s\n",
               get, path, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("    testing null '%s'\n", set);
    al.commonattr = 0;
    if (xsetattrlist(mode, path, fd, &al, &abw, sizeof(abw), 0)) {
      printf("      *** null %s() for '%s' failed: %s\n",
             set, path, strerror(errno));
      ret = 1;
    }

    if (verbose) printf("    testing null/nobuf '%s'\n", set);
    al.commonattr = 0;
    if (xsetattrlist(mode, path, fd, &al, &abw, 0, 0)) {
      printf("      *** null/nobuf %s() for '%s' failed: %s\n",
             set, path, strerror(errno));
      ret = 1;
    }
  } while (0);

  if (fd >= 0) (void) close(fd);

	if (verbose) printf("  deleting '%s'\n", path);
	if (unlink(path)) {
		printf("    *** error deleting '%s': %s (%d)\n",
					 path, strerror(errno), errno);
	}

  return ret;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  char tpath[MAXPATHLEN];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(tpath, sizeof(tpath), "%s/%s-%u", TEST_TEMP, progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  ret = do_tests(0, tpath, verbose);
  ret |= do_tests(1, tpath, verbose);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
