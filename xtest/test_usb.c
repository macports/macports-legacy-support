/* Test that USB.h can be successfully included */

/*
 * Extra hacks to allow testing with "reverse mismatched" SDK.
 *
 * Building for 10.15 with a <10.12 SDK fails due to referencing a
 * missing definition for uuid_t.  This situation is extremely unlikely
 * to occur "for real", so we only fix it in the test, just so that we don't
 * need to exclude this test from the allowable "reverse mismatch" tests.
 *
 * Similarly, we provide a missing (empty) definition for building
 * 26.x+ with a <12.x SDK.
 */

#if defined(_MACPORTS_LEGACY_MIN_EARLY_SDK_ALLOWED) \
    && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 101500 \
    && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 110000

#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 101200

#include <sys/_types.h>
typedef __darwin_uuid_t uuid_t;

#endif  /* __MPLS_SDK_MAJOR < 101200 */

#endif  /* 10.15 target */

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 260000

#ifndef __kernel_ptr_semantics
#define __kernel_ptr_semantics
#endif

#endif  /* 26.x+ target */

#endif  /* Older SDK allowed and known target */

#include <IOKit/usb/USB.h>
#include <stdio.h>

int main() {
    printf("Including <IOKit/usb/USB.h> succeeded\n");
    return 0;
}
