// freeware lmao

// MP support header
#include "MacportsLegacySupport.h"
#if __MPLS_LIB_SUPPORT_CCRANDOMGENERATEBYTES__

#include <stdlib.h>

int CCRandomGenerateBytes(void *buf, size_t nbytes) {
	arc4random_buf(buf, nbytes);
	return 0;
}

#endif // too short no need to write it again
