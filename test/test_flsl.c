/*
 * Copyright (c) 2020 Chris Jones
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

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int
main(int argc, char *argv[])
{
  int ret = 0, verbose = 0;
  const int offset = 4;

  if (argc > 1 && !strcmp(argv[1], "-v")) verbose = 1;

  /* FIXME: Actually check the results for correctness */

  if (verbose) printf( "testing fls :-\n" );
  for ( int i = 0; i <= 8*sizeof(int)-offset; i+=offset ) {
    const int ii = i+offset;
    int test = ( i>0  ? 1UL << (i-1)  : 0 );
    test |=    ( ii>0 ? 1UL << (ii-1) : 0 );
    const int j = fls(test);
    if (verbose) printf( "  Set bits %i,%i - Found bit %i\n", i, ii, j );
  }

  if (verbose) printf( "testing flsl :-\n" );
  for ( int i = 0; i <= 8*sizeof(long int)-offset; i+=offset ) {
    const int ii = i+offset;
    long int test = ( i>0  ? 1UL << (i-1)  : 0 );
    test |=         ( ii>0 ? 1UL << (ii-1) : 0 );
    const int j = flsl(test);
    if (verbose) printf( "  Set bits %i,%i - Found bit %i\n", i, ii, j );
  }

  if (verbose) printf( "testing flsll :-\n" );
  for ( int i = 0; i <= 8*sizeof(long long int)-offset; i+=offset ) {
    const int ii = i+offset;
    long long int test = ( i>0  ? 1UL << (i-1)  : 0 );
    test |=              ( ii>0 ? 1UL << (ii-1) : 0 );
    const int j = flsll(test);
    if (verbose) printf( "  Set bits %i,%i - Found bit %i\n", i, ii, j );
  }
  
  printf("%s %s.\n", basename(argv[0]), ret ? "failed" : "succeeded");
  return ret;
}
