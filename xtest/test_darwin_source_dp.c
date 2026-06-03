/*
 * Version of test_darwin_source with _POSIX_C_SOURCE and _DARWIN_C_SOURCE
 * defined.
 */

#define _POSIX_C_SOURCE (199309L-1)
#define _DARWIN_C_SOURCE 1

#include "test_darwin_source.c"
