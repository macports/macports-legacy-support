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
 * This provides some tests of the futimens() and utimensat() functions.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

typedef struct timespec timespec_t;
typedef struct timeval timeval_t;

/* Test cases copied from the old test */
static const timespec_t tptr[][2] = {
	{ { 0x12345678, 987654321 }, { 0x15263748, 123456789 }, },

	{ { 0, UTIME_NOW }, { 0x15263748, 123456789 }, },
	{ { 0x12345678, 987654321 }, { 0, UTIME_NOW }, },
	{ { 0, UTIME_NOW }, { 0, UTIME_NOW }, },

	{ { 0, UTIME_OMIT }, { 0x15263748, 123456789 }, },
	{ { 0x12345678, 987654321 }, { 0, UTIME_OMIT }, },
	{ { 0, UTIME_OMIT }, { 0, UTIME_OMIT }, },

	{ { 0, UTIME_NOW }, { 0, UTIME_OMIT }, },
	{ { 0, UTIME_OMIT }, { 0, UTIME_NOW }, },
};

static int
create_file(const char *name, int verbose, int *fdp)
{
  int fd;

  if ((fd = open(name, O_CREAT | O_RDWR,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    printf("  *** creating '%s' failed: %s (%d)\n",
           name, strerror(errno), errno);
    return 1;
  }
  if (verbose) printf("    created '%s'\n", name);

  if (fdp) {
    *fdp = fd;
    return 0;
  }
  if (close(fd)) {
    printf("  *** error closing '%s': %s (%d)\n",
           name, strerror(errno), errno);
    return 1;
  }
  return 0;
}

static void
print_case(const timespec_t *tts, const timespec_t *ots, const timespec_t *nts)
{
  printf("      {%ld, %ld}: {%ld, %ld} -> {%ld, %ld}\n",
         tts->tv_sec, tts->tv_nsec,
         ots->tv_sec, ots->tv_nsec, nts->tv_sec, nts->tv_nsec);
}

static int
do_tests(int mode, const char *path, int apfs,
         int verbose, int keepgoing, int *filter)
{
  int ret = 0, i, lret, fd = -1;
  struct stat pre_st, post_st;
  timespec_t now;

  if (verbose) printf("  Testing %s\n", mode ? "futimens()" : "utimensat()");

  ret = create_file(path, verbose, mode ? &fd : NULL);
  if (ret) return ret;

  do {
    for (i = 0; i < sizeof(tptr)/sizeof(tptr[0]); i++) {
      if (verbose) printf("    === {%ld, %ld} {%ld, %ld} ===\n",
                          tptr[i][0].tv_sec, tptr[i][0].tv_nsec,
                          tptr[i][1].tv_sec, tptr[i][1].tv_nsec);

      if (clock_gettime(CLOCK_REALTIME, &now)) {
        printf("      *** clock_gettime() failed: %s (%d)\n",
               strerror(errno), errno);
        ret = 1;
        break;
      }

      if (mode ? fstat(fd, &pre_st) : stat(path, &pre_st)) {
        printf("      *** first stat() failed: %s (%d)\n",
               strerror(errno), errno);
        ret = 1;
        break;
      }
      if (mode ? futimens(fd, tptr[i])
          : utimensat(AT_FDCWD, path, tptr[i], 0)) {
        printf("      *** %s() failed: %s (%d)\n",
               mode ? "futimens" : "utimensat", strerror(errno), errno);
        ret = 1;
        break;
      }
      if (mode ? fstat(fd, &post_st) : stat(path, &post_st)) {
        printf("      *** second stat() failed: %s (%d)\n",
               strerror(errno), errno);
        ret = 1;
        break;
      }

      /* First do the atimes */
      lret = 0;
      if (tptr[i][0].tv_nsec == UTIME_NOW) {
        if (post_st.st_atimespec.tv_sec < now.tv_sec) {
          printf("      *** post-stat atime seconds < now\n");
          lret = 1;
        }
      } else if (tptr[i][0].tv_nsec == UTIME_OMIT) {
        if (post_st.st_atimespec.tv_sec != pre_st.st_atimespec.tv_sec
            || post_st.st_atimespec.tv_nsec != pre_st.st_atimespec.tv_nsec) {
          printf("      *** atime inappropriately changed\n");
          lret = 1;
        }
      } else {
        /* Seconds must always match. */
        if (post_st.st_atimespec.tv_sec != tptr[i][0].tv_sec) {
          if (!apfs && post_st.st_atimespec.tv_sec == pre_st.st_atimespec.tv_sec
              && post_st.st_atimespec.tv_nsec == pre_st.st_atimespec.tv_nsec
              && tptr[i][1].tv_nsec == UTIME_OMIT) {
            /*
             * A bug (non-APFS only, including in Apple's code) fails to set
             * the atime when the mtime operand is set to UTIME_OMIT.  We check
             * for that case here, and tolerate the error.
             */
            if (!*filter || verbose) {
              printf("      *** "
                     "tolerating known bug setting atime without mtime.\n");
            }
            ++*filter;
          } else {
            printf("      *** new atime seconds mismatched intended value.\n");
            lret = 1;
          }
        } else {
          /*
           * Nanoseconds must match on APFS, but other filesystems may not
           * support subseconds at all, so we allow it if before and after
           * were both zero.
           */
          if (post_st.st_atimespec.tv_nsec != tptr[i][0].tv_nsec) {
            if (!apfs
                && pre_st.st_atimespec.tv_nsec == 0
                && post_st.st_atimespec.tv_nsec == 0) {
              if (verbose > 1) {
                printf("      *** allowing non-APFS zero atime nanos.\n");
              }
            } else {
              printf("      *** "
                     "new atime nanoseconds mismatched intended value.\n");
              lret = 1;
            }
          }
        }
      }
      if (lret) {
        print_case(&tptr[i][0], &pre_st.st_atimespec, &post_st.st_atimespec);
      }
      ret |= lret;
      if (ret && !keepgoing) break;

      /* Now do the mtimes */
      lret = 0;
      if (tptr[i][1].tv_nsec == UTIME_NOW) {
        if (post_st.st_mtimespec.tv_sec < now.tv_sec) {
          printf("      *** post-stat mtime seconds < now\n");
          lret = 1;
        }
      } else if (tptr[i][1].tv_nsec == UTIME_OMIT) {
        if (post_st.st_mtimespec.tv_sec != pre_st.st_mtimespec.tv_sec
            || post_st.st_mtimespec.tv_nsec != pre_st.st_mtimespec.tv_nsec) {
          printf("      *** mtime inappropriately changed\n");
          lret = 1;
        }
      } else {
        /* Seconds must always match. */
        if (post_st.st_mtimespec.tv_sec != tptr[i][1].tv_sec) {
          printf("      *** new mtime seconds mismatched intended value.\n");
          lret = 1;
        } else {
          /* See above regarding nanoseconds matching */
          if (post_st.st_mtimespec.tv_nsec != tptr[i][1].tv_nsec) {
            if (!apfs
                && pre_st.st_mtimespec.tv_nsec == 0
                && post_st.st_mtimespec.tv_nsec == 0) {
              if (verbose > 1) {
                printf("      *** allowing non-APFS zero mtime nanos.\n");
              }
            } else {
              printf("      *** "
                     "new mtime nanoseconds mismatched intended value.\n");
              lret = 1;
            }
          }
        }
      }
      if (lret) {
        print_case(&tptr[i][1], &pre_st.st_mtimespec, &post_st.st_mtimespec);
      }

      ret |= lret;
      if (ret && !keepgoing) break;
    }
		if (ret && !keepgoing) break;
  } while (0);

  if (fd >= 0 && close(fd)) {
    printf("  *** error closing '%s': %s (%d)\n",
           path, strerror(errno), errno);
  }

  /* If stopping on error, leave the file for perusal, else delete it */
  if (!ret || keepgoing) {
    if (unlink(path)) {
      printf("    *** error deleting '%s': %s (%d)\n",
             path, strerror(errno), errno);
    } else {
      if (verbose) printf("    deleted '%s'\n", path);
    }
  }

  return ret;
}

int
main(int argc, char *argv[])
{
  int argn = 1, verbose = 0, keepgoing = 0;
  int ret = 0, apfs = 0, filter = 0;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  const char *cp;
  char chr;
  struct statfs sfs = { 0 };
  char tpath[MAXPATHLEN];

  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 'K': ++keepgoing; break;
        case 'v': ++verbose; break;
      }
    }
    ++argn;
  }

  (void) snprintf(tpath, sizeof(tpath), TEST_TEMP "/%s-%u", progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  if (statfs(TEST_TEMP, &sfs)) {
    printf("  *** statfs() for '" TEST_TEMP "' failed: %s (%d)\n",
           strerror(errno), errno);
    ret = 1;
  } else {
    if (verbose) printf("  filesystem type is '%s'\n", sfs.f_fstypename);
    apfs = !strcmp(sfs.f_fstypename, "apfs");

    ret = do_tests(0, tpath, apfs, verbose, keepgoing, &filter);
    if (!ret || keepgoing) {
      ret = do_tests(1, tpath, apfs, verbose, keepgoing, &filter);
    }
  }

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
