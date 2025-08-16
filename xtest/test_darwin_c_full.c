/*
 * Version of test_darwin_c with __DARWIN_C_LEVEL set to __DARWIN_C_FULL-1.
 */

/* Set __DARWIN_C_LEVEL indirectly (can't be set directly) */
/* Use assumed __DARWIN_C_FULL value */
#define _POSIX_C_SOURCE (900000L-1)

#include "test_darwin_c.c"
