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
 * This test is just a minimal test to verify the include of net/if_utun.h.
 *
 * It's a poorly written header that fails to include other headers that
 * it needs, so we have to add them.  This has nothing to do with the
 * pre-10.6 issue.
 */

#include <stdio.h>

#include <sys/socket.h>  /* For struct sockaddr_storage in 10.8 */
#include <sys/types.h>   /* For u_int64_t */

#include <net/if_utun.h>
 
int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("net/if_utun.h successfully included\n");

  return 0;
}
