/*
 * Version of no-fix test_packet with NOCANCEL variant.
 */

#define _MACPORTS_LEGACY_DISABLE_CMSG_FIXES 1
#define __DARWIN_NON_CANCELABLE 1

#include "../test/test_packet.c"
