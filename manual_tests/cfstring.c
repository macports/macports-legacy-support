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
 * This test is just a minimal sanity check of CFStringCreateWithBytesNoCopy().
 *
 * The function itself is provided by Apple, and the only missing bit we're
 * adding is the prototype in the 10.4 case.
 *
 * Note, however, that the 10.4 CoreFoundation library is 32-bit only.  In
 * the 10.4 64-bit case, we still check the include, but don't use the code.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <CoreFoundation/CFString.h>

#include <_macports_extras/targetos.h>

const char *test = "The quick brown fox";

int
main(int argc, char *argv[])
{
  CFStringRef cfstring;

  (void) argc; (void) argv;

#if __MPLS_TARGET_OSVER < 1050 && defined(__LP64__)
  (void) cfstring;
  printf("  *** Skipping cfstring test on 64-bit 10.4\n");
  return 0;
#else
  cfstring = CFStringCreateWithBytesNoCopy(
      NULL,
      (const uint8_t *) test, strlen(test),
      CFStringGetSystemEncoding(), 0, NULL
      );

  if (cfstring) {
    printf("CFStringCreateWithBytesNoCopy() succeeded\n");
    return 0;
  } else {
    printf("CFStringCreateWithBytesNoCopy() failed\n");
    return 1;
  }
#endif
}
