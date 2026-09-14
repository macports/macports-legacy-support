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
 * This provides a test to verify that the libutil functions are present.
 *
 * Since the implementation (for 10.4) is copied verbatim from Apple's
 * 10.5 sources, we don't bother with functional tests, and instead just
 * ensure that the function symbols are present.
 *
 * Since we never link libutil statically, we can use dlsym() to check
 * the existence.
 */

#include <dlfcn.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>

/* Defs for pointer formatting */
#if defined(__LP64__) && __LP64__
#define PTR_FMT "%016llX"
typedef unsigned long long ptrint_t;
#else
#define PTR_FMT "%08X"
typedef unsigned int ptrint_t;
#endif

const char * const func_names[] = {
    "_secure_path",
    "freemntopts",
    "getmnt_silent",
    "getmntoptnum",
    "getmntopts",
    "getmntoptstr",
    "humanize_number",
    "pidfile_close",
    "pidfile_open",
    "pidfile_remove",
    "pidfile_write",
    "properties_free",
    "properties_read",
    "property_find",
    "realhostname",
    "realhostname_sa",
    "trimdomain",
    "uu_lock",
    "uu_lock_txfr",
    "uu_lockerr",
    "uu_unlock",
    };
#define NUM_FUNCS (sizeof(func_names) / sizeof(func_names[0]))

int
main(int argc, char *argv[])
{
  int verbose = 0, ret = 0, i;
  void *func;
  char *progname = basename(argv[0]);

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  for (i = 0; i < NUM_FUNCS; ++i) {
    func = dlsym(RTLD_NEXT, func_names[i]);
    if (!func) {
      printf("  *** '_%s' not found\n", func_names[i]);
      ret = 1;
    } else if (verbose) {
      printf("  _%s = " PTR_FMT "\n", func_names[i], (ptrint_t) func);
    }
  }

  printf("%s %s.\n", progname, ret ? "failed" : "succeeded");
  return ret;
}
