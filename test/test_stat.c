/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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

/* This provides some tests of the *stat() functions. */

#include <assert.h>
#include <libgen.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>

/* Make sure we always have a "struct stat64" */
#if !__MPLS_HAVE_STAT64
struct stat64 __DARWIN_STRUCT_STAT64;
#endif

/* Structure accommodating both struct stat sizes, with padding for check */
typedef struct safe_stat_s  {
  struct stat_s {
    struct stat s;
    uint64_t pad;
  } s;
  struct stat64_s {
    struct stat64 s;
    uint64_t pad;
  } s64;
} safe_stat_t;

static safe_stat_t stat_buf, stat_copy, stat_link_copy;

static int stat_err;

static const uint64_t pad_val = 0xDEADBEEFDEADBEEFULL;

static const char *source = __FILE__;
static const char *source_link = __FILE__ "_link";

static void
stat_init(int ino64)
{
  if (!ino64) {
    stat_buf.s.pad = pad_val;
  } else {
    stat_buf.s64.pad = pad_val;
  }
}

static void
copy_stat_std(int link)
{
  safe_stat_t *dest = link ? &stat_link_copy : &stat_copy;
  dest->s = stat_buf.s;
}

static void
copy_stat_64(int link)
{
  safe_stat_t *dest = link ? &stat_link_copy : &stat_copy;
  dest->s64 = stat_buf.s64;
}

static void
copy_stat(int ino64, int link)
{
  if (!ino64) {
    copy_stat_std(link);
  } else {
    copy_stat_64(link);
  }
}

static int
ts_equal(const struct timespec *a, const struct timespec *b)
{
  return a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec;
}

static void
check_copy_std(int link)
{
  const safe_stat_t *copy = link ? &stat_link_copy : &stat_copy;

  assert(stat_buf.s.s.st_dev == copy->s.s.st_dev);
  assert(stat_buf.s.s.st_ino == copy->s.s.st_ino);
  assert(stat_buf.s.s.st_mode == copy->s.s.st_mode);
  assert(stat_buf.s.s.st_nlink == copy->s.s.st_nlink);
  assert(stat_buf.s.s.st_uid == copy->s.s.st_uid);
  assert(stat_buf.s.s.st_gid == copy->s.s.st_gid);
  assert(stat_buf.s.s.st_rdev == copy->s.s.st_rdev);
  /* Don't check atime, since this test may change it. */
  assert(ts_equal(&stat_buf.s.s.st_mtimespec, &copy->s.s.st_mtimespec));
  assert(ts_equal(&stat_buf.s.s.st_ctimespec, &copy->s.s.st_ctimespec));
#if __DARWIN_64_BIT_INO_T
  assert(ts_equal(&stat_buf.s.s.st_birthtimespec,
                  &copy->s.s.st_birthtimespec));
#endif /* __DARWIN_64_BIT_INO_T */
  assert(stat_buf.s.s.st_size == copy->s.s.st_size);
  assert(stat_buf.s.s.st_blocks == copy->s.s.st_blocks);
  assert(stat_buf.s.s.st_blksize == copy->s.s.st_blksize);
  assert(stat_buf.s.s.st_flags == copy->s.s.st_flags);
  assert(stat_buf.s.s.st_gen == copy->s.s.st_gen);
}

static void
check_copy_64(int link)
{
  const safe_stat_t *copy = link ? &stat_link_copy : &stat_copy;

  assert(stat_buf.s64.s.st_dev == copy->s64.s.st_dev);
  assert(stat_buf.s64.s.st_mode == copy->s64.s.st_mode);
  assert(stat_buf.s64.s.st_nlink == copy->s64.s.st_nlink);
  assert(stat_buf.s64.s.st_ino == copy->s64.s.st_ino);
  assert(stat_buf.s64.s.st_uid == copy->s64.s.st_uid);
  assert(stat_buf.s64.s.st_gid == copy->s64.s.st_gid);
  assert(stat_buf.s64.s.st_rdev == copy->s64.s.st_rdev);
  /* Don't check atime, since this test may change it. */
  assert(ts_equal(&stat_buf.s64.s.st_mtimespec, &copy->s64.s.st_mtimespec));
  assert(ts_equal(&stat_buf.s64.s.st_ctimespec, &copy->s64.s.st_ctimespec));
  assert(ts_equal(&stat_buf.s64.s.st_birthtimespec,
                  &copy->s64.s.st_birthtimespec));
  assert(stat_buf.s64.s.st_size == copy->s64.s.st_size);
  assert(stat_buf.s64.s.st_blocks == copy->s64.s.st_blocks);
  assert(stat_buf.s64.s.st_blksize == copy->s64.s.st_blksize);
  assert(stat_buf.s64.s.st_flags == copy->s64.s.st_flags);
  assert(stat_buf.s64.s.st_gen == copy->s64.s.st_gen);
  /* Don't check reserved fields. */
}

static void
check_copy(int ino64, int link)
{
  if (!ino64) {
    check_copy_std(link);
  } else {
    check_copy_64(link);
  }
}

static int
check_err(const char *name)
{
  if (stat_err) {
    perror(name);
    return 1;
  }
  return 0;
}

static mode_t
get_mode(int ino64)
{
  if (!ino64) {
    assert(stat_buf.s.pad == pad_val);
    return stat_buf.s.s.st_mode;
  } else {
    assert(stat_buf.s64.pad == pad_val);
    return stat_buf.s64.s.st_mode;
  }
}

int
main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *fp;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) {
    printf("%s starting.\n", basename(argv[0]));
    printf(" source = %s, link = %s\n", source, source_link);
    printf(" struct stat has %d-bit st_ino\n",
           (int) sizeof(stat_buf.s.s.st_ino) * 8);
    printf(" struct stat64 has %d-bit st_ino\n",
           (int) sizeof(stat_buf.s64.s.st_ino) * 8);
  }

  if (verbose) printf("  testing 'stat'\n");
  stat_init(0);
  stat_err = stat(source, &stat_buf.s.s);
  if (check_err("stat")) return 1;
  assert(S_ISREG(get_mode(0)) && "stat expected regular file");
  copy_stat(0, 0);

  if (verbose) printf("  testing 'stat' of link\n");
  stat_init(0);
  stat_err = stat(source_link, &stat_buf.s.s);
  if (check_err("stat of link")) return 1;
  assert(S_ISREG(get_mode(0)) && "stat of link expected regular file");
  check_copy(0, 0);

  if (verbose) printf("  testing 'lstat'\n");
  stat_init(0);
  stat_err = lstat(source_link, &stat_buf.s.s);
  if (check_err("lstat")) return 1;
  assert(S_ISLNK(get_mode(0)) && "lstat expected symlink");
  copy_stat(0, 1);

  if (verbose) printf("  testing 'fstat'\n");
  /* Use fopen() to steer clear of open()/close() variant issues. */
  assert((fp = fopen(source_link, "r")) != NULL && "open of source failed");
  stat_init(0);
  stat_err = fstat(fileno(fp), &stat_buf.s.s);
  if (check_err("fstat")) return 1;
  assert(S_ISREG(get_mode(0)) && "fstat expected regular file");
  check_copy(0, 0);
  (void) fclose(fp);

#if __MPLS_HAVE_STAT64

  if (verbose) printf("  testing 'stat64'\n");
  stat_init(1);
  stat_err = stat64(source, &stat_buf.s64.s);
  if (check_err("stat64")) return 1;
  assert(S_ISREG(get_mode(1)) && "stat64 expected regular file");
  copy_stat(1, 0);

  if (verbose) printf("  testing 'stat64' of link\n");
  stat_init(1);
  stat_err = stat64(source_link, &stat_buf.s64.s);
  if (check_err("stat64 of link")) return 1;
  assert(S_ISREG(get_mode(1)) && "stat64 of link expected regular file");
  check_copy(1, 0);

  if (verbose) printf("  testing 'lstat64'\n");
  stat_init(1);
  stat_err = lstat64(source_link, &stat_buf.s64.s);
  if (check_err("lstat64")) return 1;
  assert(S_ISLNK(get_mode(1)) && "lstat64 expected symlink");
  copy_stat(1, 1);

  if (verbose) printf("  testing 'fstat64'\n");
  /* Use fopen() to steer clear of open()/close() variant issues. */
  assert((fp = fopen(source_link, "r")) != NULL && "open of source failed");
  stat_init(1);
  stat_err = fstat64(fileno(fp), &stat_buf.s64.s);
  if (check_err("fstat64")) return 1;
  assert(S_ISREG(get_mode(1)) && "fstat64 expected regular file");
  check_copy(1, 0);
  (void) fclose(fp);

#endif /* __MPLS_HAVE_STAT64 */

  printf("%s succeeded.\n", basename(argv[0]));
  return 0;
}
