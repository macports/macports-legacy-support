
#ifndef _MACPORTS_MATH_H_
#define _MACPORTS_MATH_H_

// Include the primary system math.h
#include_next <math.h>

// MP support header
#include "MacportsLegacySupport.h"

// memmem
#if __MP_LEGACY_SUPPORT_LLROUND__
#ifdef __cplusplus
extern "C" {
#endif
  extern long long llround( double x );
#ifdef __cplusplus
}
#endif
#endif

#endif // _MACPORTS_MATH_H_
