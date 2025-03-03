/*
 * Version of test_packet with CMSG fixes disabled.
 *
 * This provides a way to test the feature for disabling the fixes.
 * Since it's only a manual test, we don't bother allowing the known
 * failures to pass.
 */

#define _MACPORTS_LEGACY_DISABLE_CMSG_FIXES 1

#include "../test/test_packet.c"
