
#ifndef _MACPORTS_WCHAR_H_
#define _MACPORTS_WCHAR_H_

// Include the primary system wchar.h
#include_next <wchar.h>

// MP support header
#include "MacportsLegacySupport.h"

// strnlen
#if __MP_LEGACY_SUPPORT_WCSDUP__
#ifdef __cplusplus
extern "C" {
#endif
  extern wchar_t * wcsdup(const wchar_t *s);
#ifdef __cplusplus
}
#endif
#endif

#endif // _MACPORTS_WCHAR_H_
