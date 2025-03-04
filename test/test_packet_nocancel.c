/*
 * Version of test_packet with NOCANCEL variant.
 */

#define __DARWIN_NON_CANCELABLE 1
#include "test_packet.c"
