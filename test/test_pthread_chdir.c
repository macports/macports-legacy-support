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
 * This provides tests of the pthtread_chdir_np() and pthread_fchdir_np()
 * functions.
 *
 * The general approach is to use comparisions of stat structures to
 * determine whether files or directories are the same.  This is used
 * both to verify that changing the cwd in the test thread doesn't
 * affect the cwd in the main thread, and also that accesses relative
 * to the new cwd are as expected.
 */

#if !defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
    || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050

#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc;

  printf("%s is unsupported on 10.4\n", basename(argv[0]));
  return 0;
}

#else /* Not 10.4 */

/* Enable the prototypes */
#define _MACPORTS_LEGACY_PTHREAD_CHDIR 1

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

typedef struct info_s {
  char *argv0, *progname, *progdir;
  int cwd_fd, progdir_fd;
  int done, stop, chdir_errno, progdir_errno, progname_errno;
  struct stat cwd_sb, progdir_sb, progname_sb;
  struct stat test_cwd_sb, test_progdir_sb, test_progname_sb;
} info_t;

typedef void *(thread_fn_t)(void *);

static int
run_thread(thread_fn_t *func, info_t *ip)
{
  int err = 0;
  pthread_t thread;

  /* Start the thread */
  ip->done = ip->stop = 0;
  if (pthread_create(&thread, NULL, func, ip)) return errno;

  /* Wait for thread to do its stuff */
  while (!ip->done) usleep(1000);

  /* Now see what our cwd is */
  if (stat(".", &ip->test_cwd_sb) < 0) err = errno;

  /* Stop the test thread */
  ip->stop = 1;
  if (pthread_join(thread, NULL) && !err) err = errno;

  /* Restore our cwd just in case */
  (void) fchdir(ip->cwd_fd);
  return err;
}

static int
compare_stats(const char *name, int isdir,
              const struct stat *sb1, const struct stat *sb2)
{
  int ret = 0;

  if (sb1->st_dev != sb2->st_dev) {
    printf("    %s test st_dev %d != orig st_dev %d\n", name,
           (int) sb1->st_dev, (int) sb2->st_dev); 
    ret = ret ? ret : 1;
  }
  if (sb1->st_mode != sb2->st_mode) {
    printf("    %s test st_mode %u != orig st_mode %u\n", name,
           (unsigned int) sb1->st_mode, (unsigned int) sb2->st_mode); 
    ret = ret ? ret : 2;
  }
  /*
   * Link counts for directories are actually the entry counts,
   * and can't be counted on to remain stable, especially during
   * parallel tests.
   */
  if (!isdir && sb1->st_nlink != sb2->st_nlink) {
    printf("    %s test st_nlink %u != orig st_nlink %u\n", name,
           (unsigned int) sb1->st_nlink, (unsigned int) sb2->st_nlink); 
    ret = ret ? ret : 3;
  }
  if (sb1->st_ino != sb2->st_ino) {
    printf("    %s test st_ino %llu != orig st_ino %llu\n", name,
           (unsigned long long) sb1->st_ino,
           (unsigned long long) sb2->st_ino); 
    ret = ret ? ret : 4;
  }
  return ret;
}

static int
check_results(info_t *ip)
{
  int err;
  /* Make sure thread didn't change our cwd */
  err = compare_stats("cwd", 1, &ip->cwd_sb, &ip->test_cwd_sb);
  if (err) return err;
  err = compare_stats("progdir", 1, &ip->progdir_sb, &ip->test_progdir_sb);
  if (err) return 100 + err;
  err = compare_stats("progname", 0, &ip->progname_sb, &ip->test_progname_sb);
  if (err) return 200 + err;
  return 0;
}

static void *
test_chdir(void *arg)
{
  info_t *ip = (info_t *) arg;

  ip->chdir_errno = ip->progdir_errno = ip->progname_errno = 0;
  /* Do the actual test operation */
  if (pthread_chdir_np(ip->progdir)) ip->chdir_errno = errno;

  /* Collect the stats from the test dir & name */
  if (stat(".", &ip->test_progdir_sb) < 0) ip->progdir_errno = errno;
  if (stat(ip->progname, &ip->test_progname_sb) < 0) ip->progname_errno = errno;

  /* Tell caller we're finished, and wait to be stopped */
  ip->done = 1;
  while (!ip->stop) usleep(1000);
  return NULL;
}

static void *
test_fchdir(void *arg)
{
  info_t *ip = (info_t *) arg;

  ip->chdir_errno = ip->progdir_errno = ip->progname_errno = 0;
  /* Do the actual test operation */
  if (pthread_fchdir_np(ip->progdir_fd)) ip->chdir_errno = errno;

  /* Collect the stats from the test dir & name */
  if (stat(".", &ip->test_progdir_sb) < 0) ip->progdir_errno = errno;
  if (stat(ip->progname, &ip->test_progname_sb) < 0) ip->progname_errno = errno;

  /* Tell caller we're finished, and wait to be stopped */
  ip->done = 1;
  while (!ip->stop) usleep(1000);
  return NULL;
}

int
main(int argc, char *argv[])
{
  int verbose = 0;
  int err, ret = 0;
  info_t info;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  if (!(info.argv0 = strdup(argv[0]))
      || !(info.progname = strdup(basename(argv[0])))
      || !(info.progdir = strdup(dirname(argv[0])))) {
    perror("Can't copy program name");
    return 20;
  }

  if (verbose) printf("%s starting.\n", info.progname);

  /* Collect initial information */
  if (stat(info.argv0, &info.progname_sb) < 0) {
    perror("Can't stat program");
    return 20;
  }
  if (verbose) printf("  stat() of '%s' successful\n", info.argv0);
  if ((info.cwd_fd = open(".", O_RDONLY)) < 0) {
    perror("Can't open cwd");
    return 20;
  }
  if (fstat(info.cwd_fd, &info.cwd_sb) < 0) {
    perror("Can't stat cwd");
    return 20;
  }
  if (verbose) printf("  stat() of '.' successful\n");
  if ((info.progdir_fd = open(info.progdir, O_RDONLY)) < 0) {
    perror("Can't open program's dir");
    return 20;
  }
  if (fstat(info.progdir_fd, &info.progdir_sb) < 0) {
    perror("Can't stat program's dir");
    return 20;
  }
  if (verbose) printf("  stat() of '%s' successful\n", info.progdir);

  /* Do the chdir test */
  if (verbose) printf("  Testing pthread_chdir_np()\n");
  err = run_thread(test_chdir, &info);
  if (err) {
    printf("  some test operation failed: %s\n", strerror(err));
  } else {
    err = check_results(&info);
    if (err) {
      printf("  mismatched stat after pthread_chdir_np(), code = %d\n", err);
      ret = 1;
    }
  }

  /* Do the fchdir test */
  if (verbose) printf("  Testing pthread_fchdir_np()\n");
  err = run_thread(test_fchdir, &info);
  if (err) {
    printf("  some test operation failed: %s\n", strerror(err));
  } else {
    err = check_results(&info);
    if (err) {
      printf("  mismatched stat after pthread_fchdir_np(), code = %d\n", err);
      ret = 1;
    }
  }

  printf("%s %s\n", info.progname, ret ? "failed" : "passed");
  free(info.argv0); free(info.progname); free(info.progdir);
  return ret;
}

#endif /* Not 10.4 */
