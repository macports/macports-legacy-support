/*
 * Version of test_allheaders with _POSIX_C_SOURCE set to __DARWIN_C_FULL-1,
 * and _DARWIN_C_SOURCE defined.
 */

/* Use assumed __DARWIN_C_FULL value */
#define _POSIX_C_SOURCE (900000L-1)
#define _DARWIN_C_SOURCE

#include "test_allheaders.c"
