
#ifndef _MACPORTS_STRING_H_
#define _MACPORTS_STRING_H_

// Include the primary system string.h
#include_next <string.h>

// MP support header
#include "MacportsLegacySupport.h"

// strnlen
#if __MP_LEGACY_SUPPORT_STRNLEN__
#ifdef __cplusplus
extern "C" {
#endif
  extern size_t strnlen(const char *s, size_t maxlen);
#ifdef __cplusplus
}
#endif
#endif

// strndup
#if __MP_LEGACY_SUPPORT_STRNDUP__
#ifdef __cplusplus
extern "C" {
#endif
  extern char *strndup(const char *s, size_t n);
#ifdef __cplusplus
}
#endif
#endif

// memmem
#if __MP_LEGACY_SUPPORT_MEMMEM__
#ifdef __cplusplus
extern "C" {
#endif
  extern void *
  memmem(const void *l, size_t l_len, const void *s, size_t s_len);
#ifdef __cplusplus
}
#endif
#endif

#endif // _MACPORTS_STRING_H_
