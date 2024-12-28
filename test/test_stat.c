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

#include <sys/fcntl.h>
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
static char dir[MAXPATHLEN], rel_base[MAXPATHLEN], rel_link[MAXPATHLEN];

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

static void
setup_names(void)
{
  char *cp;
  char temp[MAXPATHLEN];

  /* Don't assume non-clobbering dirname() */
  (void) strncpy(temp, source, sizeof(temp));
  cp = dirname(temp);
  assert(cp && "dirname failed");
  (void) strncpy(dir, cp, sizeof(dir));

  (void) strncpy(temp, source_link, sizeof(temp));
  cp = dirname(temp);
  assert(cp && "dirname failed");
  do {
    if (strncmp(cp, dir, sizeof(dir))) break;

    (void) strncpy(temp, source, sizeof(temp));
    cp = basename(temp);
    if (!cp) break;
    (void) strncpy(rel_base, cp, sizeof(rel_base));

    (void) strncpy(temp, source_link, sizeof(temp));
    cp = basename(temp);
    if (!cp) break;
    (void) strncpy(rel_link, cp, sizeof(rel_link));

    return;
  } while(0);

  (void) strncpy(dir, ".", sizeof(dir));
  (void) strncpy(rel_base, source, sizeof(rel_base));
  (void) strncpy(rel_link, source_link, sizeof(rel_link));
}

int
main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *fp; int fd;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (verbose) {
    printf("%s starting.\n", basename(argv[0]));
    printf(" source = %s, source_link = %s\n", source, source_link);
  }

  setup_names();

  if (verbose) {
    printf(" dir = %s, rel_base = %s, rel_link = %s\n",
           dir, rel_base, rel_link);
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

  /* Use fopen() to steer clear of open()/close() variant issues. */
  assert((fp = fopen(dir, "r")) != NULL && "open of source failed");
  fd = fileno(fp);

  if (verbose) printf("  testing 'fstatat' (AT_FDCWD)\n");
  stat_init(0);
  stat_err = fstatat(AT_FDCWD, source, &stat_buf.s.s, 0);
  if (check_err("fstatat (AT_FDCWD)")) return 1;
  assert(S_ISREG(get_mode(0)) && "fstatat (AT_FDCWD) expected regular file");
  check_copy(0, 0);

  if (verbose) printf("  testing 'fstatat' (AT_FDCWD) of link\n");
  stat_init(0);
  stat_err = fstatat(AT_FDCWD, source_link, &stat_buf.s.s, 0);
  if (check_err("fstatat (AT_FDCWD) of link")) return 1;
  assert(S_ISREG(get_mode(0))
         && "fstatat (AT_FDCWD) of link expected regular file");
  check_copy(0, 0);

  if (verbose) printf("  testing 'fstatat' (AT_FDCWD) of link (NOFOLLOW)\n");
  stat_init(0);
  stat_err = fstatat(AT_FDCWD, source_link, &stat_buf.s.s,
                     AT_SYMLINK_NOFOLLOW);
  if (check_err("fstatat (AT_FDCWD) (NOFOLLOW)")) return 1;
  assert(S_ISLNK(get_mode(0))
         && "fstatat (AT_FDCWD) (NOFOLLOW) expected symlink");
  check_copy(0, 1);


  if (verbose) printf("  testing 'fstatat' (dir)\n");
  stat_init(0);
  stat_err = fstatat(fd, rel_base, &stat_buf.s.s, 0);
  if (check_err("fstatat (dir)")) return 1;
  assert(S_ISREG(get_mode(0)) && "fstatat (dir) expected regular file");
  check_copy(0, 0);

  if (verbose) printf("  testing 'fstatat' (dir) of link\n");
  stat_init(0);
  stat_err = fstatat(fd, rel_link, &stat_buf.s.s, 0);
  if (check_err("fstatat (dir) of link")) return 1;
  assert(S_ISREG(get_mode(0)) && "fstatat (dir) of link expected regular file");
  check_copy(0, 0);

  if (verbose) printf("  testing 'fstatat' (dir) of link (NOFOLLOW)\n");
  stat_init(0);
  stat_err = fstatat(fd, rel_link, &stat_buf.s.s,
                     AT_SYMLINK_NOFOLLOW);
  if (check_err("fstatat (dir) (NOFOLLOW)")) return 1;
  assert(S_ISLNK(get_mode(0)) && "fstatat (dir) (NOFOLLOW) expected symlink");
  check_copy(0, 1);

#if __MPLS_HAVE_STAT64

/*
 * The fstatat64 function is not expected to be accessed directly (though many
 * system libraries provide it as a convenience synonym for fstatat$INODE64),
 * so no SDK provide a prototype for it.  We do so here.
 */

  extern int fstatat64(int fd, const char *path,
                       struct stat64 *buf, int flag);

  if (verbose) printf("  testing 'fstatat64' (AT_FDCWD)\n");
  stat_init(1);
  stat_err = fstatat64(AT_FDCWD, source, &stat_buf.s64.s, 0);
  if (check_err("fstatat64 (AT_FDCWD)")) return 1;
  assert(S_ISREG(get_mode(1)) && "fstatat64 (AT_FDCWD) expected regular file");
  check_copy(1, 0);

  if (verbose) printf("  testing 'fstatat64' (AT_FDCWD) of link\n");
  stat_init(1);
  stat_err = fstatat64(AT_FDCWD, source_link, &stat_buf.s64.s, 0);
  if (check_err("fstatat64 (AT_FDCWD) of link")) return 1;
  assert(S_ISREG(get_mode(1))
         && "fstatat64 (AT_FDCWD) of link expected regular file");
  check_copy(1, 0);

  if (verbose) printf("  testing 'fstatat64' (AT_FDCWD) of link (NOFOLLOW)\n");
  stat_init(1);
  stat_err = fstatat64(AT_FDCWD, source_link, &stat_buf.s64.s,
                       AT_SYMLINK_NOFOLLOW);
  if (check_err("fstatat64 (AT_FDCWD) (NOFOLLOW)")) return 1;
  assert(S_ISLNK(get_mode(1))
         && "fstatat64 (AT_FDCWD) (NOFOLLOW) expected symlink");
  check_copy(1, 1);


  if (verbose) printf("  testing 'fstatat64' (dir)'\n");
  stat_init(1);
  stat_err = fstatat64(fd, rel_base, &stat_buf.s64.s, 0);
  if (check_err("fstatat64 (dir)")) return 1;
  assert(S_ISREG(get_mode(1)) && "fstatat64 (dir) expected regular file");
  check_copy(1, 0);

  if (verbose) printf("  testing 'fstatat64' (dir) of link\n");
  stat_init(1);
  stat_err = fstatat64(fd, rel_link, &stat_buf.s64.s, 0);
  if (check_err("fstatat64 (dir) of link")) return 1;
  assert(S_ISREG(get_mode(1))
         && "fstatat64 (dir) of link expected regular file");
  check_copy(1, 0);

  if (verbose) printf("  testing 'fstatat64' (dir) of link (NOFOLLOW)\n");
  stat_init(1);
  stat_err = fstatat64(fd, rel_link, &stat_buf.s64.s,
                       AT_SYMLINK_NOFOLLOW);
  if (check_err("fstatat64 (dir) (NOFOLLOW)")) return 1;
  assert(S_ISLNK(get_mode(1)) && "fstatat64 (dir) (NOFOLLOW) expected symlink");
  check_copy(1, 1);

#endif /* __MPLS_HAVE_STAT64 */

  (void) fclose(fp);

  printf("%s succeeded.\n", basename(argv[0]));
  return 0;
}