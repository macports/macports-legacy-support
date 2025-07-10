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
       printf("  *** type/creator lengths are not 4/4\n");
       return 1;
    }
    memset(ab, 0, sizeof(*ab));
    memcpy(&ab->finderInfo[0], FINFO_TYPE, FINFO_TCSIZE);
    memcpy(&ab->finderInfo[4], FINFO_CREATOR, FINFO_TCSIZE);
    return 0;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, fd = -1, err;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  char tpath[MAXPATHLEN];
  attrlist_t al = {.bitmapcount = ATTR_BIT_MAP_COUNT};
  rFInfoAttrBuf_t abr; FInfoAttrBuf_t abw;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(tpath, sizeof(tpath), "%s/%s-%u", TEST_TEMP, progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  do {
    if (verbose) printf("  creating '%s'\n", tpath);
    if ((fd = open(tpath, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
       printf("*** unable to open '%s': %s\n", tpath, strerror(errno));
       ret = 1;
       break;
    }

    if (verbose) printf("  testing 'getattrlist'\n");
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    if (getattrlist(tpath, &al, &abr, sizeof(abr), 0)) {
      printf("  *** getattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    } else {
      if (abr.length != sizeof(abr)) {
        printf("  *** getattrlist() returned length %d, should be %d\n",
               abr.length, (int) sizeof(abr));
        ret = 1;
      }
      if (abr.objType != VREG) {
        printf("  *** '%s' is not a regular file.\n", tpath);
        ret = 1;
      }

      if (verbose) printf("  testing 'setattrlist'\n");
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      if (setattrlist(tpath, &al, &abw, sizeof(abw), 0)) {
        printf("  *** setattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      } 

      if (verbose) printf("  testing nobuf 'setattrlist'\n");
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      err = setattrlist(tpath, &al, &abw, 0, 0);
      if (err) {
        if (errno == ENOMEM || errno == EINVAL) {
          if (verbose) printf("    failed as expected: %s\n", strerror(errno));
        } else {
          printf("  *** nobuf setattrlist() for '%s' failed: %s\n",
                 tpath, strerror(errno));
          ret = 1;
        }
      } else {
        printf("  *** nobuf setattrlist() unexpectedly succeeded\n");
        ret = 1;
      }
    }

    if (verbose) printf("  testing null 'getattrlist'\n");
    al.commonattr = 0;
    abr.length = -1;
    if (getattrlist(tpath, &al, &abr, sizeof(abr), 0)) {
      printf("  *** null getattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    } else {
      /* Null result should have length of just the length field alone. */
      if (abr.length != sizeof(abr.length)) {
        printf("  *** null getattrlist() returned length %d, should be %d\n",
               abr.length, (int) sizeof(abr.length));
        ret = 1;
      }
    }

    if (verbose) printf("  testing nobuf 'getattrlist'\n");
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    /* Silent truncation means "success" */
    err = getattrlist(tpath, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("    failed as expected: %s\n", strerror(errno));
      } else {
        printf("  *** nobuf getattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("  testing null/nobuf 'getattrlist'\n");
    al.commonattr = 0;
    abr.length = -1;
    err = getattrlist(tpath, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("    failed as expected: %s\n", strerror(errno));
      } else {
        printf("  *** null/nobuf getattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("  testing null 'setattrlist'\n");
    al.commonattr = 0;
    if (setattrlist(tpath, &al, &abw, sizeof(abw), 0)) {
      printf("  *** null setattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    }

    if (verbose) printf("  testing null/nobuf 'setattrlist'\n");
    al.commonattr = 0;
    if (setattrlist(tpath, &al, &abw, 0, 0)) {
      printf("  *** null/nobuf setattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    }

    if (verbose) printf("  testing 'fgetattrlist'\n");
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    if (fgetattrlist(fd, &al, &abr, sizeof(abr), 0)) {
      printf("  *** fgetattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    } else {
      if (abr.length != sizeof(abr)) {
        printf("  *** fgetattrlist() returned length %d, should be %d\n",
               abr.length, (int) sizeof(abr));
        ret = 1;
      }
      if (abr.objType != VREG) {
        printf("  *** '%s' is not a regular file.\n", tpath);
        ret = 1;
      }

      if (verbose) printf("  testing 'fsetattrlist'\n");
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      if (fsetattrlist(fd, &al, &abw, sizeof(abw), 0)) {
        printf("  *** fsetattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      } 

      if (verbose) printf("  testing nobuf 'fsetattrlist'\n");
      al.commonattr = ATTR_CMN_FNDRINFO;
      if (set_tc(&abw)) {
        ret = 1;
        break;
      }
      err = fsetattrlist(fd, &al, &abw, 0, 0);
      if (err) {
        if (errno == ENOMEM || errno == EINVAL) {
          if (verbose) printf("    failed as expected: %s\n", strerror(errno));
        } else {
          printf("  *** nobuf fsetattrlist() for '%s' failed: %s\n",
                 tpath, strerror(errno));
          ret = 1;
        }
      } else {
        printf("  *** nobuf fsetattrlist() unexpectedly succeeded\n");
        ret = 1;
      }
    }

    if (verbose) printf("  testing null 'fgetattrlist'\n");
    al.commonattr = 0;
    abr.length = -1;
    if (fgetattrlist(fd, &al, &abr, sizeof(abr), 0)) {
      printf("  *** null fgetattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    } else {
      /* Null result should have length of just the length field alone. */
      if (abr.length != sizeof(abr.length)) {
        printf("  *** null fgetattrlist() returned length %d, should be %d\n",
               abr.length, (int) sizeof(abr.length));
        ret = 1;
      }
    }

    if (verbose) printf("  testing nobuf 'fgetattrlist'\n");
    al.commonattr = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;
    abr.length = -1;
    /* Silent truncation means "success" */
    err = fgetattrlist(fd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("    failed as expected: %s\n", strerror(errno));
      } else {
        printf("  *** nobuf fgetattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("  testing null/nobuf 'fgetattrlist'\n");
    al.commonattr = 0;
    abr.length = -1;
    err = fgetattrlist(fd, &al, &abr, 0, 0);
    if (err) {
      if (errno == EINVAL || errno == ERANGE) {
        if (verbose) printf("    failed as expected: %s\n", strerror(errno));
      } else {
        printf("  *** null/nobuf fgetattrlist() for '%s' failed: %s\n",
               tpath, strerror(errno));
        ret = 1;
      }
    }

    if (verbose) printf("  testing null 'fsetattrlist'\n");
    al.commonattr = 0;
    if (fsetattrlist(fd, &al, &abw, sizeof(abw), 0)) {
      printf("  *** null fsetattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    }

    if (verbose) printf("  testing null/nobuf 'fsetattrlist'\n");
    al.commonattr = 0;
    if (fsetattrlist(fd, &al, &abw, 0, 0)) {
      printf("  *** null/nobuf fsetattrlist() for '%s' failed: %s\n",
             tpath, strerror(errno));
      ret = 1;
    }
  } while (0);

  if (fd >= 0) (void) close(fd);
  (void) unlink(tpath);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
