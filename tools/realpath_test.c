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

/*
 * This provides a purely manual realpath test, for investigative purposes.
 * It reports the results for a given input, for all available versions,
 * with and without a supplied output buffer.  It always reports the results
 * of the unadulterated OS functions, and optionally reports the results of
 * the functions provided by a specified library.
 */

#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>  /* For NULL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#include <malloc/malloc.h>  /* For malloc_size() */

/* RTLD_FIRST is unavailable on 10.4.  It's probably unimportant, anyway. */
#ifndef RTLD_FIRST
#define RTLD_FIRST 0
#endif

/* Function type for realpath() */
typedef char *rp_func_t(const char * __restrict, char * __restrict);

typedef struct rp_entry_s {
  int local;
  const char *name;
  rp_func_t *func;
} rp_entry_t;

static rp_entry_t funcs[] = {
  {0, "realpath", NULL},
  {0, "realpath$UNIX2003", NULL},
  {0, "realpath$DARWIN_EXTSN", NULL},
  {1, "realpath", NULL},
  {1, "realpath$UNIX2003", NULL},
  {1, "realpath$DARWIN_EXTSN", NULL},
};
#define NUM_FUNCS (sizeof(funcs) / sizeof(funcs[0]))

typedef struct sig_entry_s {
  int sig;
  const char *name;
} sig_entry_t;

static sig_entry_t sigs[] = {
  {SIGBUS, "Bus error"},
  {SIGSEGV, "Segmentation violation"},
};
#define NUM_SIGS (sizeof(sigs) / sizeof(sigs[0]))

static void *lib_handle = NULL;

static int
load_lib(const char *path, int verbose)
{
  lib_handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
  if (!lib_handle) return 1;
  if (verbose) {
    printf("  Loaded %s at handle 0x%0lX\n", path, (unsigned long) lib_handle);
  }
  return 0;
}

static void
unload_lib(void)
{
  if (!lib_handle) return;
  if (dlclose(lib_handle)) {
    fprintf(stderr, "Can't close library: %s\n", dlerror());
  }
  lib_handle = NULL;
}

static void
find_sym(rp_entry_t *ent, int verbose)
{
  void *handle = ent->local ? lib_handle : RTLD_NEXT;
  const char *type = ent->local ? "lib" : "OS ";

  if (!handle) return;
  ent->func = (rp_func_t *) dlsym(handle, ent->name);

  if (verbose) {
    if (ent->func) {
      printf("    Found:     %s %s at 0x%0lX\n",
             type, ent->name, (unsigned long) ent->func);
    } else {
      printf("    Not found: %s %s\n", type, ent->name);
    }
  }
}

static void
find_syms(int verbose)
{
  int i;

  for (i = 0; i < NUM_FUNCS; ++i) {
    find_sym(&funcs[i], verbose);
  }
}

static const char *
get_signame(int sig)
{
  int i;

  for (i = 0; i < NUM_SIGS; ++i) {
    if (sigs[i].sig == sig) return sigs[i].name;
  }
  return NULL;
}

/*
 * Some cases we test may crash with bad memory accesses.  We want to catch
 * those without crashing the entire program.  The straightforward way
 * to do that would be with a signal handler, but merely returning from
 * the signal handler (after recording the signal) repeats the bad access,
 * ad infinitum, and bailing from the handler via longjmp() isn't safe in
 * this context.  So our simplest alternative is to run the test call in
 * a subprocess, which then abnormally exits in any crash case.
 *
 * If we were to perform the test "for real" in a subprocess, we'd need
 * to set up a shared communication page to capture the result for the
 * main process.  In lieu of that, we use the subprocess for a "dry run",
 * and then repeat the test in the main process if the dry run didn't crash.
 */
static void
try_case(rp_entry_t *ent, const char *path, int no_buf)
{
  const char *type = ent->local ? "lib" : "OS ";
  char *result;
  pid_t child, done;
  int status;
  const char *signame;
  char buf[PATH_MAX];

  if (!ent->func) return;

  printf("  %s %s (buf%s supplied) for '%s':\n",
         type, ent->name, no_buf ? " not" : "", path);

  /* First a "dry run" in a subprocess, in case it crashes. */
  fflush(stdout);  /* Forking with unflushed buffers may cause trouble. */
  child = fork();
  if (child < 0) {
    perror("fork() failed");
    return;
  }
  if (child == 0) {
    result = (*ent->func)(path, no_buf ? NULL : buf);
    /* Leave any allocated buffer for the process exit to clean up. */
    exit(0);
  }
  done = wait(&status);
  if (done != child) {
    fprintf(stderr, "***** Unexpected wait() pid, %d != %d\n", done, child);
    return;
  }
  if (status) {
    if(!(signame = get_signame(status))) signame = "(unknown)";
    printf("    ***** crashed with exit status %d (%s)\n", status, signame);
    return;
  }

  /* Now do it for real */
  errno = 0;
  result = (*ent->func)(path, no_buf ? NULL : buf);

  if (result) {
    if (errno) {
      printf("    ***** 'success' with errno = %d (%s)\n",
             errno, strerror(errno));
    }
    if (!no_buf && result != buf) {
      printf("    ***** returned buffer adr 0x%0lX != supplied adr 0x%0lx\n",
             (unsigned long) result, (unsigned long) buf);
    }

    printf("    '%s'\n", result);

    /*
     * There is a bug in the 32-bit 10.6 non-POSIX realpath(), where some
     * cases that would normally be errors instead "succeed" with a pointer
     * to an internal buffer (if one wasn't supplied by the caller), rather
     * than one from malloc().  This buffer is probably unsafe to reference
     * in general (though we do it above, apparently successfully), and
     * cannot be freed with free(), thereby violating the API.
     *
     * We check for and report this case here.
     */
    if (no_buf) {
      if (!malloc_size(result)) {
        printf("    ***** returned buffer adr 0x%0lX is not from malloc()\n",
               (unsigned long) result);
      } else {
        free(result);
      }
    }
  } else {
    printf("    failed, errno = %d (%s)\n", errno, strerror(errno));
  }
}

static void
try_all(const char *path)
{
  int i;

  for (i = 0; i < NUM_FUNCS; ++i) {
    try_case(&funcs[i], path, 0);
  }
  for (i = 0; i < NUM_FUNCS; ++i) {
    try_case(&funcs[i], path, 1);
  }
}

int
main(int argc, char *argv[])
{
  int argn = 1, verbose = 0;
  const char *lib_path = NULL;

  while (argn < argc) {
    if (!strcmp(argv[argn], "-h")) {
      printf("Usage is: %s [-v] [-l <library path>] <test path>...\n",
             basename(argv[0]));
      return 0;
    }
    if (!strcmp(argv[argn], "-v")) {
      verbose = 1;
      ++argn;
      continue;
    }
    if (!strcmp(argv[argn], "-l")) {
      ++argn;
      if (argn < argc) {
        lib_path = argv[argn];
        ++argn;
      } else {
        fprintf(stderr, "-l needs library path arg\n");
        return 10;
      }
      continue;
    }
    break;
  }

  if (lib_path && load_lib(lib_path, verbose)) {
    fprintf(stderr, "Can't open library %s: %s\n", lib_path, dlerror());
    return 20;
  }

  find_syms(verbose);

  if (argn >= argc) {
    printf(" Defaulting test case to '.'\n");
    try_all(".");
  } else {
    while (argn < argc) {
      try_all(argv[argn]);
      ++argn;
    }
  }

  unload_lib();

  return 0;
}
