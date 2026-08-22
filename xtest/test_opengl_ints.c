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
 * Five integer types defined in OpenGL/AGL are typedef'ed differently in
 * the 10.4 version that later versions, sometimes confusing code that
 * expects the later types.  We provide an overlay for the offending
 * header to correct this.  This test verifies the expected 10.5+ types.
 *
 * Since even benign typdedef redefs may provoke warnings, we use function
 * argument types to provide type checks, though this usually only results
 * in a warning in the failure case.  We also check the size of a struct
 * containing the types, but the int vs. long mismatch only occurs in 64-bit
 * builds.
 *
 * The flag _MPLS_USE_AGL applies the test to AGL instead of OpenGL,
 * which is only applicable on <=10.6.  If this flag is set for 10.7+,
 * we convert the test to a dummy.
 */

#include <libgen.h>
#include <stdio.h>

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if !_MPLS_USE_AGL || __MPLS_SDK_MAJOR < 1070

#define GL_SILENCE_DEPRECATION 1

#if !_MPLS_USE_AGL
  #include <OpenGL/gl.h>
  #define LIB_NAME "OpenGL"
#else
  #include <AGL/gl.h>
  #define LIB_NAME "AGL"
#endif

/* Types as provided by the header */
typedef struct glints_s {
  GLenum x_enum;
  GLbitfield x_bitfield;
  GLint x_int;
  GLsizei x_sizei;
  GLuint x_uint;
} glints_t;

/* Types as expected in 10.5+ */
typedef struct ex_glints_s {
  unsigned int x_enum;
  unsigned int x_bitfield;
  int x_int;
  int x_sizei;
  unsigned int x_uint;
} ex_glints_t;

/* Function using provided types */
typedef void (glint_fn_t)(GLenum, GLbitfield, GLint, GLsizei, GLuint);

/* Function using expected types */
typedef void (ex_glint_fn_t)
             (unsigned int, unsigned int, int, int, unsigned int);

glint_fn_t *glint_fn_p;
ex_glint_fn_t *ex_glint_fn_p;

int
main(int argc, char *argv[])
{
  int ret;

  (void) argc; (void) argv;

  glint_fn_p = ex_glint_fn_p;
  if ((ret = sizeof(glints_t) != sizeof(ex_glints_t))) {
    printf("  *** " LIB_NAME " types mismatch\n");
  }

  printf("%s %s\n", basename(argv[0]), ret ? "failed" : "succeeded");
  return ret;
}

#else  /* AGL && >= 10.7 */

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("%s is inapplicable to 10.7+\n", basename(argv[0]));
  return 0;
}

#endif  /* AGL && >= 10.7 */
