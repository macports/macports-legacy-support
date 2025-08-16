/*
 * Version of test_realpath for compatibility entry.
 */

/*
 * This tests the macports_legacy_realpath() compatibility entry.
 *
 * For OS versions that never used the wrapper, this entire test is a dummy.
 * Because of that condition, this test is an exception to the rule that tests
 * shouldn't check the feature flags.
 */

/* MP support header */
#include "MacportsLegacySupport.h"
#if __MPLS_LIB_SUPPORT_REALPATH_ALLOC__

#define TEST_MACPORTS_LEGACY_REALPATH

#include "test_realpath.c"

#else /* !__MPLS_LIB_SUPPORT_REALPATH_ALLOC__ */

int main(){ return 0; }

#endif /* !__MPLS_LIB_SUPPORT_REALPATH_ALLOC__ */
