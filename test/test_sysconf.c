/*
 * Copyright (c) 2019 Christian Cornelssen
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
 * Deliberately declaring some potentially redefined names
 * before including the associated header file, to test robustness.
 */

/* Wrapper should work not only with calls, but with references as well */
typedef long (*itol_t)(int);

/* Renaming different objects should not affect functionality */
typedef struct { long sysconf; } scv_t;
typedef struct { itol_t sysconf; } scf_t;

#include <assert.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    int ret = 0, verbose = 0;
    char *progname = basename(argv[0]);

    if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

    if (verbose) printf("%s started\n", progname);
    /* Test with direct function call */
    long nconf = sysconf(_SC_NPROCESSORS_CONF);
    long nonln = sysconf(_SC_NPROCESSORS_ONLN);
    long nphyspages = sysconf(_SC_PHYS_PAGES);
    if (verbose) printf("  nconf = %ld; nonln = %ld;\n", nconf, nonln);
    assert (nconf > 0);
    assert (nonln > 0);
    if (verbose) printf("  sysconf(_SC_NPROCESSORS_XXXX) supported.\n");

    if (verbose) printf("  nphyspages = %ld\n", nphyspages);
    assert (nphyspages > 0);
    if (verbose) printf("    sysconf(_SC_PHYS_PAGES) supported.\n");
    if (verbose) printf("    Total system memory = %f GiB\n",
                        ((double)sysconf(_SC_PHYS_PAGES))
                        * ((double)sysconf(_SC_PAGESIZE)/(1024*1024*1024)));

    /* Test with name (reference) only */
    {
        itol_t f = sysconf;
        assert (f(_SC_NPROCESSORS_CONF) == nconf);
        assert (f(_SC_NPROCESSORS_ONLN) == nonln);
        if (verbose) printf("  f = sysconf, f(_SC_NPROCESSORS_XXXX)"
                            " supported.\n");

        assert (f(_SC_PHYS_PAGES) == nphyspages);
        if (verbose) printf("  f = sysconf, f(_SC_PHYS_PAGES) supported.\n");
    }

    /* Test with function macro disabler */
    assert ((sysconf)(_SC_NPROCESSORS_CONF) == nconf);
    assert ((sysconf)(_SC_NPROCESSORS_ONLN) == nonln);
    if (verbose) printf("  (sysconf)(_SC_NPROCESSORS_XXXX) supported.\n");

    assert ((sysconf)(_SC_PHYS_PAGES) == nphyspages);
    if (verbose) printf("  (sysconf)(_SC_PHYS_PAGES) supported.\n");

    /* Test with same-named fields */
    {
        scf_t scf = { sysconf };
        scv_t scv;
        scv.sysconf = scf.sysconf(_SC_NPROCESSORS_CONF);
        assert (scv.sysconf == nconf);
        scv.sysconf = scf.sysconf(_SC_NPROCESSORS_ONLN);
        assert (scv.sysconf == nonln);
        scv.sysconf = scf.sysconf(_SC_PHYS_PAGES);
        assert (scv.sysconf == nphyspages);
        if (verbose) printf("  scv.sysconf = scf.sysconf(_SC_NPROCESSORS_XXXX)"
                            " supported.\n");
        if (verbose) printf("  scv.sysconf = scf.sysconf(_SC_PHYS_PAGES)"
                            " supported.\n");
    }

    printf("%s %s.\n", progname, ret ? "failed" : "passed");
    return ret;
}
