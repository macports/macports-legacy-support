/*
 * Copyright (c) 2019 Mihai Moldovan <ionic@ionic.de>
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

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/syslimits.h>
#include <errno.h>
#include <fcntl.h>

#define FEEDBACK 1


int main (int argc, char **argv) {
#if !__DARWIN_ONLY_64_BIT_INO_T && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
  int saved_errno = errno;

  /* Caveat: dirname() and basename() my modify their arguments, so create copies. */
  char *prog = strdup (argv[0]);
  saved_errno = errno;

  if (!prog) {
    fprintf (stderr, "Unable to copy program name: %s\nBailing out.\n", strerror (saved_errno));

    return (EXIT_FAILURE);
  }

  char *prog_dirname = strdup (dirname (prog));
  saved_errno = errno;
  free (prog);
  prog = NULL;

  if (!prog_dirname) {
    fprintf (stderr, "Unable to copy program's dirname: %s\nBailing out.\n", strerror (saved_errno));

    return (EXIT_FAILURE);
  }

  prog = strdup (argv[0]);
  saved_errno = errno;

  if (!prog) {
    fprintf (stderr, "Unable to copy program name: %s\nBailing out.\n", strerror (saved_errno));

    free (prog_dirname);

    return (EXIT_FAILURE);
  }

  char *prog_basename = strdup (basename (prog));
  saved_errno = errno;
  free (prog);
  prog = NULL;

  if (!prog_basename) {
    fprintf (stderr, "Unable to copy program's basename: %s\nBailing out.\n", strerror (saved_errno));

    free (prog_dirname);

    return (EXIT_FAILURE);
  }

  /* Fetch raw stat64 data. */
  struct stat64 orig_stat64 = { 0 };
  if (0 != stat64 (argv[0], &orig_stat64)) {
    saved_errno = errno;
    fprintf (stderr, "Unable to get test file's stat64 information: %s\nBailing out.\n", strerror (saved_errno));

    free (prog_dirname);
    free (prog_basename);

    return (EXIT_FAILURE);
  }

  /* Test stat64-compatibility with AT_FDCWD, which should behave exactly the same. */
  struct stat64 atfdcwd_stat64 = { 0 };
  if (0 != fstatat64 (AT_FDCWD, argv[0], &atfdcwd_stat64, 0)) {
    saved_errno = errno;
    fprintf (stderr, "Unable to get test file's stat64 information via fstatat64() in AT_FDCWD mode: %s\nBailing out.\n", strerror (saved_errno));

    free (prog_dirname);
    free (prog_basename);

    return (EXIT_FAILURE);
  }

  if (0 != memcmp (&orig_stat64, &atfdcwd_stat64, sizeof (struct stat64))) {
    fprintf (stderr, "Results obtained by stat64() and fstatat64() in AT_FDCWD mode deviate.\nBailing out.\n");

    free (prog_dirname);
    free (prog_basename);

    return (EXIT_FAILURE);
  }

  /*
   * Now we actually want to use a directory that is different from our CWD.
   * If the dirname is not ".", we're pretty much done.
   *
   * Otherwise, more work is required.
   */
  struct stat64 othercwd_stat64 = { 0 };
  int prog_dir_fd = -1;
  if ((1 == strlen (prog_dirname)) && ('.' == prog_dirname[0])) {
    /*
     * The binary sits in our CWD (or has been called differently).
     * We'll fetch the CWD, extract the last component, switch to the upper
     * directory and use the extracted component in our fstatat64 call.
     */
    const size_t max_size = 1024000;
    size_t cur_size = PATH_MAX;
    char *cwd = NULL;
    char *ret = NULL;

    while (NULL == ret) {
      char *realloc_ret = realloc (cwd, cur_size);
      if (NULL == realloc_ret) {
        fprintf (stderr, "Unable to request more memory to store the CWD in.\nBailing out.\n");

        free (cwd);
        free (prog_dirname);
        free (prog_basename);

        return (EXIT_FAILURE);
      }

      cwd = realloc_ret;
      ret = getcwd (cwd, cur_size);
      saved_errno = errno;
      if ((NULL == ret) && (ERANGE != saved_errno)) {
        fprintf (stderr, "Unable to fetch CWD: %s\nBailing out.\n", strerror (saved_errno));

        /* Do not free cwd here, it's state is explicitly undefined. */
        free (prog_dirname);
        free (prog_basename);

        return (EXIT_FAILURE);
      }

      cur_size *= 2;

      if (cur_size > max_size) {
        fprintf (stderr, "CWD length would exceed maximum safe guard value of 1 MiB.\nBailing out.\n");

        free (cwd);
        free (prog_dirname);
        free (prog_basename);

        return (EXIT_FAILURE);
      }
    }

    char *cwd_copy = strdup (cwd);
    saved_errno = errno;
    if (!cwd_copy) {
      fprintf (stderr, "Unable to copy CWD: %s\nBailing out.\n", strerror (saved_errno));

      free (cwd);
      free (prog_dirname);
      free (prog_basename);

      return (EXIT_FAILURE);
    }

    char *last_component = strdup (basename (cwd_copy));
    saved_errno = errno;
    free (cwd_copy);
    cwd_copy = NULL;
    free (cwd);
    cwd = NULL;

    if (!last_component) {
      fprintf (stderr, "Unable to copy CWD's basename: %s\nBailing out.\n", strerror (saved_errno));

      free (prog_dirname);
      free (prog_basename);

      return (EXIT_FAILURE);
    }

    if (0 != chdir ("..")) {
      saved_errno = errno;
      fprintf (stderr, "Unable to change to upper directory: %s\nBailing out.\n", strerror (saved_errno));

      free (last_component);
      free (prog_dirname);
      free (prog_basename);
    }

    /* Now get a reference to the previous directory. */
    prog_dir_fd = open (last_component, O_RDONLY | O_DIRECTORY);
    saved_errno = errno;

    if (-1 == prog_dir_fd) {
      fprintf (stderr, "Unable to open directory '%s' from upper dir: %s\nBailing out.\n", last_component, strerror (saved_errno));

      free (last_component);
      free (prog_dirname);
      free (prog_basename);

      return (EXIT_FAILURE);
    }
  }
  else {
    /* Our CWD differs, so we can just open a reference to dirname. */
    prog_dir_fd = open (prog_dirname, O_RDONLY | O_DIRECTORY);
    saved_errno = errno;

    if (-1 == prog_dir_fd) {
      fprintf (stderr, "Unable to open directory '%s': %s\nBailing out.\n", prog_dirname, strerror (saved_errno));

      free (prog_dirname);
      free (prog_basename);

      return (EXIT_FAILURE);
    }
  }

  int fstatat64_ret = fstatat64 (prog_dir_fd, prog_basename, &othercwd_stat64, 0);
  saved_errno = errno;
  close (prog_dir_fd);
  prog_dir_fd = -1;
  free (prog_dirname);
  prog_dirname = NULL;
  free (prog_basename);
  prog_basename = NULL;

  /* Done, populate struct (and check for errors, naturally). */
  if (0 != fstatat64_ret) {
    fprintf (stderr, "Unable to get test file's stat64 information via fstatat64() with a different CWD supplied: %s\nBailing out.\n", strerror (saved_errno));

    return (EXIT_FAILURE);
  }

  if (0 != memcmp (&orig_stat64, &othercwd_stat64, sizeof (struct stat64))) {
    fprintf (stderr, "Results obtained by stat64() and fstatat64() with a different CWD supplied deviate.\nBailing out.\n");

    return (EXIT_FAILURE);
  }
#else
  printf ("\n\nfstatat64() is not supported on your platform, this test will do nothing.\n\n");
#endif

  return (EXIT_SUCCESS);
}
