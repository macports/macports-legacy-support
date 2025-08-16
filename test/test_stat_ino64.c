/*
 * Version of test_stat with 64-bit inodes (if possible).
 *
 * This is primarily for 10.5, where 64-bit inodes are supported
 * but not the default.
 */

#define _DARWIN_USE_64_BIT_INODE 1

#include "test_stat.c"
