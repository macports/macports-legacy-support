/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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
 * This provides some basic tests of the extended [f]chmodx() functions, mainly
 * because they need fixes for 10.4.
 *
 * We test a simple modification of the normal mode, since that's easy
 * and within the scope of legacy-support's partial fix.
 *
 * If an alternate gid is available, we also test modifying the gid.  We
 * don't test modifying the uid, since that would create complications.
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "filesec_internal.h"

#ifndef TEST_TEMP
#define TEST_TEMP "/dev/null"
#endif

#define MAX_GROUPS 200

static int
check_statx(struct _filesec *fs1, struct _filesec *fs2)
{
  int valid;

  if (fs1->fs_valid != fs2->fs_valid) return 1;
  valid = fs1->fs_valid;
  if (valid & FS_VALID_UID && fs1->fs_uid != fs2->fs_uid) return 1;
  if (valid & FS_VALID_GID && fs1->fs_gid != fs2->fs_gid) return 1;
  if (valid & FS_VALID_UUID
      && bcmp(&fs1->fs_uuid, &fs2->fs_uuid, sizeof(fs1->fs_uuid))) return 1;
  if (valid & FS_VALID_GRPUUID
      && bcmp(&fs1->fs_grpuuid, &fs2->fs_grpuuid, sizeof(fs1->fs_grpuuid))) {
    return 1;
  }
  if (valid & FS_VALID_MODE && fs1->fs_mode != fs2->fs_mode) return 1;
  if (valid & FS_VALID_ACL && fs1->fs_aclsize != fs2->fs_aclsize) return 1;
  /* Don't bother checking fs_aclbuf */
  return 0;
}

/* Get alternate group for testing */
/* This can be any of our groups other than the default */
static gid_t
get_testgrp(void)
{
  int err, i;
  uid_t ouruid = getuid();
  gid_t basegid, cur;
  struct passwd *pwd;
  int ngroups = MAX_GROUPS;
  int groups[MAX_GROUPS];

  /* We need our actual userid to use getgrouplist() */
  pwd = getpwuid(ouruid);
  if (!pwd) return 0;
  basegid = pwd->pw_gid;

  err = getgrouplist(pwd->pw_name, basegid, groups, &ngroups);
  /* Some errors "succeed" and don't store ngroups */
  if (err || ngroups == MAX_GROUPS) return 0;
  for (i = 0; i < ngroups; ++i) {
    cur = groups[i];
    if (cur != basegid) return cur;
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, fd = -1, err, fdmode;
  char *progname = basename(argv[0]);
  pid_t pid = getpid();
  struct _filesec *fsec = NULL;  /* a.k.a. filesec_t */
  struct _filesec *fsec_mod = NULL, *fsec_check = NULL;
  gid_t testgid;
  struct stat sb;
  char tpath[PATH_MAX];

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  (void) snprintf(tpath, sizeof(tpath), "%s/%s-%u", TEST_TEMP, progname, pid);

  if (verbose) printf("%s starting.\n", progname);

  testgid = get_testgrp();

  do {
    if (verbose) printf("  creating '%s'\n", tpath);
    if ((fd = open(tpath, O_CREAT | O_RDWR, S_IRWXU)) < 0) {
       printf("  *** unable to open '%s': %s\n", tpath, strerror(errno));
       ret = 1;
       break;
    }

    if (verbose) printf("  allocating a 'filesec_t'\n");
    fsec = filesec_init();
    if (!fsec) {
      perror("  *** filesec_init() failed");
      ret = 1;
      break;
    }
    if (verbose) printf("  allocating another 'filesec_t'\n");
    fsec_check = filesec_init();
    if (!fsec_check) {
      perror("  *** filesec_init() failed");
      ret = 1;
      break;
    }

    for (fdmode = 0; fdmode < 2; ++fdmode) {

      if (verbose) printf("  getting test file status"
                          " for %schmodx_np() mode test\n",
                          fdmode ? "f" : "");
      err = fstatx_np(fd, &sb, fsec);
      if (err) {
        perror("    *** fstatx_np() failed");
        ret = 1;
        continue;
      }

      if (verbose) printf("    duplicating and modifying status\n");
      if (fsec_mod) filesec_free(fsec_mod);
      fsec_mod = filesec_dup(fsec);
      if (!fsec) {
        perror("    *** filesec_dup() failed");
        ret = 1;
        continue;
      }

      fsec_mod->fs_mode ^= S_IROTH;  /* Complement world read for test */

      if (verbose) printf("    setting mode with %schmodx_np()\n",
                          fdmode ? "f" : "");
      if (!fdmode) {
        err = chmodx_np(tpath, fsec_mod);
      } else {
        err = fchmodx_np(fd, fsec_mod);
      }
      if (err) {
        perror(fdmode ? "    *** fchmodx_np() failed"
                      : "    *** chmodx_np() failed");
        ret = 1;
        continue;
      }

      if (verbose) printf("    getting updated status\n");
      err = fstatx_np(fd, &sb, fsec_check);
      if (err) {
        perror("    *** fstatx_np() failed");
        ret = 1;
        continue;
      }

      if (verbose) printf("    checking status\n");
      err = check_statx(fsec_check, fsec_mod);
      if (err) {
        printf("  *** updated status mismatched\n");
        ret = 1;
        continue;
      }

      if (!testgid) {
        printf("  *** no alternate gid available for gid test\n");
        continue;
      }

      fsec_mod->fs_gid = testgid;

      if (verbose) printf("    setting alternate gid with %schmodx_np()\n",
                          fdmode ? "f" : "");
      if (!fdmode) {
        err = chmodx_np(tpath, fsec_mod);
      } else {
        err = fchmodx_np(fd, fsec_mod);
      }
      if (err) {
        perror(fdmode ? "    *** fchmodx_np() failed"
                      : "    *** chmodx_np() failed");
        ret = 1;
        continue;
      }

      if (verbose) printf("    getting updated status\n");
      err = fstatx_np(fd, &sb, fsec_check);
      if (err) {
        perror("    *** fstatx_np() failed");
        ret = 1;
        continue;
      }

      if (verbose) printf("    checking status\n");
      err = check_statx(fsec_check, fsec_mod);
      if (err) {
        printf("  *** updated status mismatched\n");
        ret = 1;
        continue;
      }

      if (verbose) printf("  %schmodx succeeded\n", fdmode ? "f" : "");
    }
  } while (0);

  if (fsec_check) filesec_free(fsec_check);
  if (fsec_mod) filesec_free(fsec_mod);
  if (fsec) filesec_free(fsec);
  if (fd >= 0) (void) close(fd);
  if (verbose) printf("  deleting '%s'\n", tpath);
  (void) unlink(tpath);

  printf("%s %s.\n", progname, ret ? "failed" : "passed");
  return ret;
}
