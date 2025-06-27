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
 * This is a tool for reporting some basic "stat" info about files, including
 * the timestamps with full resolution as reported.
 *
 * This tool is intended to be built without legacy-support, but can
 * optionally load the legacy-support library from either the "system"
 * location or the relative build-tree location.  In the former case,
 * the default "/opt/local" prefix is assumed, unless the MPPREFIX
 * definition is overridden.
 */

#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef __DARWIN_SUF_64_BIT_INO_T
#define __DARWIN_SUF_64_BIT_INO_T
#endif

/* RTLD_FIRST is unavailable on 10.4 - make it ignored. */
#ifndef RTLD_FIRST
#define RTLD_FIRST 0
#endif

#if defined(__ppc__)
#define ARCH "ppc"
#elif defined(__ppc64__)
#define ARCH "ppc64"
#elif defined(__i386__)
#define ARCH "i386"
#elif defined(__x86_64__)
#define ARCH "x86_64"
#elif defined(__arm__)
#define ARCH "arm"
#elif defined(__arm64__) || defined(__aarch64__)
#define ARCH "arm64"
#else
#define ARCH "unknown"
#endif

#define SYSCTL_OSVER_CLASS CTL_KERN
#define SYSCTL_OSVER_ITEM  KERN_OSRELEASE

/* sysctl to check whether we're running natively (not Rosetta 1) */
#define SYSCTL_NATIVE "sysctl.proc_native"

/* sysctl to check whether we're running in Rosetta 2 */
#define SYSCTL_TRANSLATED "sysctl.proc_translated"

typedef int (sysctl_fn_t)(int *name, u_int namelen,
             void *oldp, size_t *oldlenp, void *newp, size_t newlen);
typedef int (sysctlbyname_fn_t)(const char *name,
             void *oldp, size_t *oldlenp, void *newp, size_t newlen);
typedef int (sysctlnametomib_fn_t)(const char *name, int *mibp, size_t *sizep);

#ifndef MPPREFIX
#define MPPREFIX "/opt/local"
#endif
#define LIBDIR "lib"
#define LSLIB "libMacportsLegacySupport.dylib"
#define MPLSLIB MPPREFIX "/" LIBDIR "/" LSLIB
#define LOCALLSLIB "../" LIBDIR "/" LSLIB

#define UL (unsigned long)
#define LL (long long)
#define ULL (unsigned long long)
#define BILLION 1000000000ULL

static char osver[256];

static int
get_osver(void)
{
  int mib[] = {SYSCTL_OSVER_CLASS, SYSCTL_OSVER_ITEM};
  int miblen = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(osver);

  if (sysctl(mib, miblen, osver, &len, NULL, 0)) return -1;
  if (len <= 0 || len >= (ssize_t) sizeof(osver)) return -1;
  osver[len] = '\0';
  if (osver[len - 1] == '\n') osver[len - 1] = '\0';
  return 0;
}

/* Test whether running under Rosetta */
/* -1 no, 1 Rosetta 1, 2 Rosetta 2 */
static int
check_rosetta(void)
{
  int native, translated;
  size_t native_sz = sizeof(native);
  size_t translated_sz = sizeof(translated);

  /* Check for Rosetta 1 */
  if (sysctlbyname(SYSCTL_NATIVE, &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    return -1;
  }
  if (!native) return 1;

  /* If "native", check for Rosetta 2 */
  if (sysctlbyname(SYSCTL_TRANSLATED, &translated, &translated_sz,
                   NULL, 0) < 0) {
    /* If sysctl failed, must be really native. */
    return -1;
  }
  return translated ? 2 : -1;
}

static void *
load_lib(int legacy, char *progname, int verbose)
{
  char *progdir;
  char lslib[PATH_MAX], lsreal[PATH_MAX];
  const char *libpath;
  void *libhandle = NULL;

  if (legacy > 0) {
    libpath = MPLSLIB;
  } else {
    progdir = dirname(progname);
    (void) snprintf(lslib, sizeof(lslib), "%s/" LOCALLSLIB, progdir);
    if (!(libpath = realpath(lslib, lsreal))) {
      printf("Unable to resolve library path '%s': %s\n",
             lslib, strerror(errno));
      return NULL;
    }
  }
  if (!(libhandle = dlopen(libpath, RTLD_FIRST))) {
    printf("Unable to open library: %s\n", dlerror());
    return NULL;
  }
  if (verbose) {
    printf("    Loaded %s, handle = 0x%0*lX\n", libpath,
           (int) sizeof(void *) * 2, UL libhandle);
  }
  return libhandle;
}

static void
close_lib(void **libhandle)
{
  if (*libhandle && dlclose(*libhandle)) {
    printf("Unable to close library: %s\n", dlerror());
  }
  *libhandle = NULL;
}

static void *
func_lookup(const char *name, void *libhandle, int verbose)
{
  void *handle = libhandle;
  void *adr = NULL;

  /* Try extra library first, then general */
  if (libhandle) adr = dlsym(handle, name);
  if (!adr) adr = dlsym((handle = RTLD_NEXT), name);

  if (!adr) {
    printf("    %s not found\n", name);
    return NULL;
  }

  if (verbose) {
    if (handle == RTLD_NEXT) {
      printf("    Located %s in library handle RTLD_NEXT\n", name);
    } else {
      printf("    Located %s in library handle 0x%0*lX\n", name,
             (int) sizeof(void *) * 2, UL libhandle);
    }
  }

  return adr;
}

typedef __typeof__(lstat) lstat_fn_t;

static void
report_type(mode_t mode)
{
  const char *type ="(unknown)";

  switch (mode & S_IFMT) {
    case S_IFIFO: type = "fifo"; break;
    case S_IFCHR: type = "character special"; break;
    case S_IFDIR: type = "directory"; break;
    case S_IFBLK: type = "block special"; break;
    case S_IFREG: type = "regular"; break;
    case S_IFLNK: type = "symbolic link"; break;
    case S_IFSOCK: type = "socket"; break;
    case S_IFWHT: type = "whiteout"; break;
  }
  printf("  type is %s\n", type);
}

static void
report_time(const char *name, struct timespec *ts)
{
  if (ULL ts->tv_nsec < BILLION) {
    printf("  %s = %lld.%09lld\n", name, LL ts->tv_sec, LL ts->tv_nsec);
  } else {
    printf("  %s tv_sec = %lld, bad tv_nsec = %lld (0x%08llX)\n",
           name, LL ts->tv_sec, LL ts->tv_nsec, ULL ts->tv_nsec);
  }
}

static void
report_stat(const char *name, lstat_fn_t *os_lstat, int verbose)
{
  int err;
  struct stat sb;
  char rpath[MAXPATHLEN];

  printf("lstat() for '%s':\n", name);
  if (verbose) {
    if (!realpath(name, rpath)) {
      printf("  *** realpath() failed at %s: %s (%d)\n",
             rpath, strerror(errno), errno);
    } else {
      printf("  full path is %s\n", rpath);
    }
  }
  err = (*os_lstat)(name, &sb);
  if (err) {
    printf("  *** failed: %s (%d)\n", strerror(errno), errno);
    return;
  }
  report_type(sb.st_mode);
  report_time("atime    ", &sb.st_atimespec);
  report_time("mtime    ", &sb.st_mtimespec);
  report_time("ctime    ", &sb.st_ctimespec);
#ifdef _DARWIN_FEATURE_64_BIT_INODE
  report_time("birthtime", &sb.st_birthtimespec);
#endif
}

int
main(int argc, char *argv[])
{
  int ret, err, verbose = 0, legacy = 0, argn = 1;
  const char *cp;
  char chr;
  void *libhandle = NULL;
  const char *rosetta;
  static lstat_fn_t *os_lstat = NULL;

  while (argn < argc && argv[argn][0] == '-') {
    cp = argv[argn];
    while ((chr = *++cp)) {
      switch (chr) {
        case 'v': ++verbose; break;
        case 'y': legacy = 1; break;
        case 'Y': legacy = -1; break;
      }
    }
    ++argn;
  }

  err = get_osver();
  switch (check_rosetta()) {
    case -1: rosetta = "native"; break;
    case 1: rosetta = "Rosetta 1"; break;
    case 2: rosetta = "Rosetta 2"; break;
    default: rosetta = "???"; break;
  }
  printf("OS is Darwin %s, CPU is %s (%s)\n",
         err ? "???" : osver, ARCH, rosetta);

  if (legacy) {
    if (!(libhandle = load_lib(legacy, argv[0], verbose))) return 10;
  }

  /* Use the correct version of lstat() for this build. */
  os_lstat = func_lookup("lstat" __DARWIN_SUF_64_BIT_INO_T, libhandle, verbose);
  if (!os_lstat) return 1;

  while (argn < argc) {
    report_stat(argv[argn], os_lstat, verbose);
    ++argn;
  }

  close_lib(&libhandle);

  return 0;
}
