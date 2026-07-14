// bawls :3

#ifndef _MACPORTS_CommonCrypto_CommonRandom_H_
#define _MACPORTS_CommonCrypto_CommonRandom_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/* Include the primary system CommonCrypto/CommonRandom.h */
#include_next <CommonCrypto/CommonRandom.h>

#if __MPLS_SDK_SUPPORT_CCommonRandom__

#define CCRandomGenerateBytes(A,B) CCRandomGenerateBytes(A,B)

#endif /* _MACPORTS_CommonCrypto_CommonRandom_H_ */

#endif /* _MACPORTS_CommonCrypto_CommonRandom_H_ */
