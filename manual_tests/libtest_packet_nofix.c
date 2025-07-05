/*
 * Version of test_packet with CMSG fixes disabled.
 *
 * This provides a way to test the feature for disabling the fixes.
 */

#define _MACPORTS_LEGACY_DISABLE_CMSG_FIXES 1

#include "../test/test_packet.c"
