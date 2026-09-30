/*
 * Version of strncpy_chk with enable forced on.
 *
 * The security wrapper mechanism is unavailable on 10.4, available but
 * defaulted off in 10.5, and enabled and defaulted on in 10.6+.
 * Overriding the default here enables it on 10.5, with no effect on other
 * OS versions.
 */

#define _FORTIFY_SOURCE 2

#include "libtest_strncpy_chk.c"
