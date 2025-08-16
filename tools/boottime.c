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
 * This is a tool for obtaining the system boot time, for the purpose
 * of investigating bugs in that mechanism.
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

#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

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

  if (verbose) {
    if (!adr) {
      printf("    %s not found\n", name);
    } else if (handle == RTLD_NEXT) {
      printf("    Located %s in library handle RTLD_NEXT\n", name);
    } else {
      printf("    Located %s in library handle 0x%0*lX\n", name,
             (int) sizeof(void *) * 2, UL libhandle);
    }
  }

  return adr;
}

static size_t
init_bt(struct timeval *bt)
{
  uint32_t *ip = (uint32_t *) bt;

  /* Prefill result with garbage, to detect incomplete stores */
  while (ip < (uint32_t *)(bt + 1)) {
    *ip++ = 0xDEADBEEFU;
  }
  return sizeof(*bt);
}

static void
print_err(const char *name, int ret, int err)
{
  printf("***  kern.boottime %s returned %d, errno = %d (%s)\n",
         name, ret, err, strerror(err));
}

static int
check_bt(const char *name, int retlen, struct timeval *bt)
{
  if (retlen != sizeof(*bt)) {
    printf("***  kern.boottime %s returned length %d, which should be %d\n",
           name, retlen, (int) sizeof(*bt));
    return 1;
  }
  if ((unsigned int) bt->tv_usec >= 1000000U) {
    if (sizeof(bt->tv_usec) > 4) {
      printf("***  kern.boottime %s tv_usec = 0x%016llX\n",
             name, (unsigned long long) bt->tv_usec);
    } else {
      printf("***  kern.boottime %s tv_usec = 0x%08X\n",
             name, (unsigned int) bt->tv_usec);
    }
    return 1;
  }
  return 0;
}

static void
print_bt(const char *name, struct timeval *bt)
{
  printf("  kern.bootime %s result = { sec = %lld, usec = %lld }\n",
         name, (long long) bt->tv_sec, (long long) bt->tv_usec);
}

static void
show_mib(const char *name, const int *mib, size_t miblen)
{
  int val;
  const int *mibend = mib + miblen;

  printf("  kern.boottime%s mib = [", name);
  while (mib < mibend) {
    val = *mib++;
    printf("%d%s", val, mib < mibend ? ", " : "");
  }
  printf("]\n");
}

static void
do_flush(void)
{
  (void) fflush(stdout);
}

int
main(int argc, char *argv[])
{
  int ret, err, verbose = 0, legacy = 0, argn = 1;
  const char *cp;
  char chr;
  void *libhandle = NULL;
  const char *rosetta;
  struct timeval boottime;
  size_t boottime_len;
  int mib[8];
  size_t miblen;

  sysctlbyname_fn_t *sysctlbyname_p;
  sysctlnametomib_fn_t *sysctlnametomib_p;
  sysctl_fn_t *sysctl_p;

  static const int bt_mib[] = {CTL_KERN, KERN_BOOTTIME};
  static const size_t bt_miblen = sizeof(bt_mib) / sizeof(bt_mib[0]);

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

  do_flush();  /* In case we crash */

  boottime_len = init_bt(&boottime);
  if ((sysctlbyname_p = func_lookup("sysctlbyname", libhandle, verbose))) {
    ret = (*sysctlbyname_p)("kern.boottime", &boottime, &boottime_len, NULL, 0);
    if (ret) {
      print_err("by name", ret, errno);
    } else {
      ret = check_bt("by name", boottime_len, &boottime);
    }
    if (!ret || verbose) {
      print_bt("by name", &boottime);
    }
  }
  do_flush();  /* In case we crash */

  miblen = sizeof(mib) / sizeof(mib[0]);
  if ((sysctlnametomib_p = 
       func_lookup("sysctlnametomib", libhandle, verbose))) {
    ret = (*sysctlnametomib_p)("kern.boottime", mib, &miblen);
    if (ret) {
      print_err("to mib", ret, errno);
      miblen = 0;
    }
    if (!ret || verbose) {
      show_mib("", mib, miblen);
    }
  } else miblen = 0;
  do_flush();  /* In case we crash */

  sysctl_p = func_lookup("sysctl", libhandle, verbose);

  if (miblen) {
    boottime_len = init_bt(&boottime);
    if (sysctl_p) {
      ret = (*sysctl_p)(mib, miblen, &boottime, &boottime_len, NULL, 0);
      if (ret) {
        print_err("by mib", ret, errno);
      } else {
        ret = check_bt("by mib", boottime_len, &boottime);
      }
      if (!ret || verbose) {
        print_bt("by mib", &boottime);
      }
    }
  }
  do_flush();  /* In case we crash */

  boottime_len = init_bt(&boottime);
  memcpy(mib, bt_mib, bt_miblen * sizeof(bt_mib[0]));
  show_mib(" by consts", mib, bt_miblen);
  do_flush();  /* In case we crash */
  if (sysctl_p) {
    ret = (*sysctl_p)(mib, bt_miblen, &boottime, &boottime_len, NULL, 0);
    if (ret) {
      print_err("by consts", ret, errno);
    } else {
      ret = check_bt("by consts", boottime_len, &boottime);
    }
    if (!ret || verbose) {
      print_bt("by consts", &boottime);
    }
  }
  do_flush();  /* In case we crash */

  close_lib(&libhandle);

  return 0;
}
