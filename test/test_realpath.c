
/*
 * Copyright (c) 2019 Christian Cornelssen
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
 * Deliberately declaring some potentially redefined names
 * before including the associated header file, to test robustness.
 */

/* Wrapper should work not only with calls, but with references as well */
/* __restrict not needed here (and remember: nothing included yet) */
typedef char * (*strfunc_t)(const char *, char *);

/* Renaming different objects should not affect functionality */
typedef struct { char *realpath; } rpv_t;
typedef struct { strfunc_t realpath; } rpf_t;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    /* Test with direct function call */
    const char *p = realpath(".", NULL);	/* bus error up to 10.5 */
    if (!p) return 1;
    printf("cwd = %s\n", p);
    printf("realpath(path, NULL) supported.\n");

    /* Test with name (reference) only */
    {
        strfunc_t f = realpath;
        const char *q = f(".", NULL);
        if (!q) return 1;
        assert (!strcmp(q, p));
        printf("f = realpath, f(path, NULL) supported.\n");
        free((void*)q);
    }

    /* Test with function macro disabler */
    {
        const char *q = (realpath)(".", NULL);
        if (!q) return 1;
        assert (!strcmp(q, p));
        printf("(realpath)(path, NULL) supported.\n");
        free((void*)q);
    }

    /* Test with same-named fields */
    {
        rpf_t rpf = { realpath };
	rpv_t rpv;
        rpv.realpath = rpf.realpath(".", NULL);
        if (!rpv.realpath) return 1;
        assert (!strcmp(rpv.realpath, p));
        printf("rpv.realpath = rpf.realpath(path, NULL) supported.\n");
        free((void*)rpv.realpath);
    }

    free((void*)p);
    return 0;
}

