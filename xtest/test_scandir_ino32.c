/*
 * Version of test_scandir with 32-bit inodes.
 */

#define __DARWIN_64_BIT_INO_T 0
#define __DARWIN_ONLY_64_BIT_INO_T 0

#include "test_scandir.c"
