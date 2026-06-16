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
 * This is a manual test to report OS and SDK versions, as well as checking
 * __MPLS_SDK_MAJOR against the SDK version supplied via the SDKVER environment
 * variable (defaulting to the target OS version), in the same format as
 * MacOSX<version>.sdk.
 */

/* Do this before everything else. */
#include <_macports_extras/sdkversion.h>

#include <_macports_extras/targetos.h>

#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include <sys/sysctl.h>

#define OS_VERSION_FILE "/System/Library/CoreServices/SystemVersion.plist"
#define OS_VERSION_KEY "[[:blank:]]*<key>ProductUserVisibleVersion</key>"
#define OS_VERSION_VAL "[[:blank:]]*<string>([0-9.]+)</string>"

#define SYSCTL_VERSION_COMPAT_NAME "kern.system_version_compat"

#define SYSCTL_KERNVER_CLASS CTL_KERN
#define SYSCTL_KERNVER_ITEM  KERN_OSRELEASE

#define OS_TARGET_ENV "MACOSX_DEPLOYMENT_TARGET"

#define SDKVER_ENV "SDKVER"

#ifdef __MPLS_TARGET_OSVER
#define TARGET_OS __MPLS_TARGET_OSVER
#else
#define TARGET_OS 0
#endif

/* Get the "version compatibility hack" status */
static int
get_version_hack_status(void)
{
  int enable, err;
  size_t enbsiz = sizeof(enable);
  err = sysctlbyname(SYSCTL_VERSION_COMPAT_NAME, &enable, &enbsiz, NULL, 0);
  return err ? 0 : enable;
}

static char osver[256];

/* Kludgy single-purpose XML parser for SystemVersion (to avoid dependencies) */
static int
get_osver(void)
{
  int ret = 0, regerr = 0, found = 0, verlen;
  regex_t *regerr_pat;
  size_t regerr_size;
  FILE *verfile = NULL;
  regex_t key_pat = {0}, val_pat = {0};
  regmatch_t matches[2];
  #define REGMATCH_N (sizeof(matches) / sizeof(matches[0]))
  char verbuf[128], *verline;
  char regerr_buf[128];

  do {
    if ((regerr = regcomp(&key_pat, OS_VERSION_KEY, 0))) {
      regerr_pat = &key_pat;
      break;
    }
    if ((regerr = regcomp(&val_pat, OS_VERSION_VAL, REG_EXTENDED))) {
      regerr_pat = &val_pat;
      break;
    }
    if (!(verfile = fopen(OS_VERSION_FILE, "r"))) {
      perror("  *** unable to open SystemVersion file");
      ret = 1;
      break;
    }
    while (1) {
      if (!(verline = fgets(verbuf, sizeof(verbuf), verfile))) {
        if (ferror(verfile)) {
          perror("  *** error reading SystemVersion file");
        } else {
          fprintf(stderr, "  *** OS version key not found\n");
        }
        ret = 1;
        break;
      }
      if (found) break;
      if ((regerr = regexec(&key_pat, verline, REGMATCH_N, matches, 0))) {
        if (regerr == REG_NOMATCH) continue;
        regerr_pat = &key_pat;
        break;
      } else {
        found = 1;
        continue;
      }
    }
    if (ret) break;
    if ((regerr = regexec(&val_pat, verline, REGMATCH_N, matches, 0))) {
      regerr_pat = &val_pat;
      break;
    }
    verlen = matches[1].rm_eo - matches[1].rm_so;
    if (matches[1].rm_so < 0 || matches[1].rm_eo < 0 || verlen < 1) {
      fprintf(stderr, "  *** bad version match\n");
      ret = 1;
      break;
    }
    /* Since verbuf is smaller than osver, this can't overflow */
    memcpy(osver, &verline[matches[1].rm_so], verlen);
    osver[verlen] = '\0';
  } while (0);

  if (regerr) {
    regerr_size = regerror(regerr, regerr_pat, regerr_buf, sizeof(regerr_buf));
    if (regerr_size > sizeof(regerr_buf)) {
      regerr_buf[sizeof(regerr_buf) - 1] = '\0';
    }
    fprintf(stderr, "  *** regex error: %s\n", regerr_buf);
    ret = 1;
  }

  if (verfile) (void) fclose(verfile);
  regfree(&val_pat);
  regfree(&key_pat);

  return ret;
}

static char kernver[256];

static int
get_kernver(void)
{
  int mib[] = {SYSCTL_KERNVER_CLASS, SYSCTL_KERNVER_ITEM};
  int miblen = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(kernver);

  if (sysctl(mib, miblen, kernver, &len, NULL, 0)) return -1;
  if (len <= 0 || len >= (ssize_t) sizeof(kernver)) return -1;
  kernver[len] = '\0';
  if (kernver[len - 1] == '\n') kernver[len - 1] = '\0';
  return 0;
}

static int
get_vernum(const char *verstr)
{
  long major, minor, micro;
  char *endp;

  if (!verstr || !*verstr) return 0;

  major = strtol(verstr, &endp, 10);
  if (*endp == '.') {
    minor = strtol(endp + 1, &endp, 10);
    if (*endp == '.') {
      micro = strtol(endp + 1, &endp, 10);
    } else {
      micro = 0;
    }
  } else {
    minor = micro = 0;
  }
  if (major < 10) return -1;
  if (*endp && (major != 10 || minor != 4 || *endp != 'u')) return -1;
  if (major == 10 && minor <= 9) {
    return (int) (major * 100 + minor * 10 + MIN(micro, 9));
  }
  return (int) (major * 10000 + minor * 100 + micro);
}

static int
get_darwin(int vernum)
{
  int major, minor;

  if (vernum >= 10000) {
    major = vernum / 10000;
    minor = vernum % 10000 / 100;
  } else {
    major = vernum / 100;
    minor = vernum % 100 / 10;
  }
  if (major < 11) return minor + 4;
  if (major < 26) return major + 9;
  return major - 1;
}

int
main(int argc, char *argv[])
{
  const char *sdkver = NULL, *target = NULL;
  int verhack, osvernum, osdarwin, targetnum, sdknum, sdkmajor, err;

  (void) argc; (void) argv;

  sdkver = getenv(SDKVER_ENV);
  sdknum = get_vernum(sdkver);
  if (sdknum < 0) {
    fprintf(stderr, "Bad SDK version: %s\n", sdkver ? sdkver : "???");
    return 20;
  }
  if (!sdknum) sdknum = TARGET_OS;
  if (sdknum < 10000) {
    sdkmajor = sdknum / 10 * 10;
  } else if (sdknum < 110000) {
    sdkmajor = sdknum / 100 * 100;
  } else {
    sdkmajor = sdknum / 10000 * 10000;
  }

  printf("Testing SDK version %s,%s numeric = %d, major = %d\n",
         sdkver ? sdkver : "<default>", sdkver ? "" : " assumed",
         sdknum, sdkmajor);

  verhack = get_version_hack_status();

  err = get_osver();
  if (err) {
    printf("  Running OS is ???\n");
  } else {
    osvernum = get_vernum(osver);
    if (osvernum < 0) {
      printf("  Running OS is %s?\n", osver);
    } else {
      osdarwin = get_darwin(osvernum);
      printf("  Running OS is %s, numeric = %d, darwin = %d\n",
             osver, osvernum, osdarwin);
      if (verhack) {
        printf("    *** Version may be a \"compatibility hack\"\n");
        printf("    *** Set SYSTEM_VERSION_COMPAT=0 to fix\n");
      }
    }
  }

  err = get_kernver();
  printf("  Running kernel is Darwin %s\n", err ? "???" : kernver);

  target = getenv(OS_TARGET_ENV);
  if (!target || !*target) {
    printf("  %s is unspecified, assuming current OS\n", OS_TARGET_ENV);
  } else {
    targetnum = get_vernum(target);
    if (targetnum < 0) {
      printf("  %s is %s?\n", OS_TARGET_ENV, target);
    } else {
      printf("  %s is %s, numeric = %d\n", OS_TARGET_ENV, target, targetnum);
    }
  }

  #ifndef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
    printf("  __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ is undefined\n");
  #else
    printf("  __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ = %d\n",
           __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__);
  #endif

  #ifndef __MPLS_TARGET_OSVER
    printf("  __MPLS_TARGET_OSVER is undefined\n");
  #else
    printf("  __MPLS_TARGET_OSVER = %d\n", __MPLS_TARGET_OSVER);
  #endif

  #ifndef _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED
    printf("  _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED is undefined\n");
  #else
    printf("  _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED = %d\n",
           _MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED);
  #endif

  #ifndef __MPLS_SDK_MAJOR
    printf("  __MPLS_SDK_MAJOR is undefined\n");
    return 1;
  #else
    if (__MPLS_SDK_MAJOR != sdkmajor) {
      printf("  __MPLS_SDK_MAJOR is %d, should be %d\n",
             __MPLS_SDK_MAJOR, sdkmajor);
      return 2;
    } else {
      printf("  __MPLS_SDK_MAJOR is correctly %d\n", sdkmajor);
    }
  #endif

  printf("\n");
  return 0;
}
