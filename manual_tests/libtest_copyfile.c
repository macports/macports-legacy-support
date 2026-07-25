/* Wrapper to build copyfile.c as a standalone test program. */

#include <_macports_extras/targetos.h>

#define _COPYFILE_TEST 1
#define _COPYFILE_DEBUG 1

#if __MPLS_TARGET_OSVER < 1050
#define _NO_QUARANTINE 1
#endif

#include "../src/copyfile.c"
