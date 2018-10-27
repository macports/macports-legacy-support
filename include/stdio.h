
#ifndef _MACPORTS_STDIO_H_
#define _MACPORTS_STDIO_H_

// Include the primary system time.h
#include_next <stdio.h>

// MP support header
#include "MacportsLegacySupport.h"

// getline
#if __MP_LEGACY_SUPPORT_GETLINE__
//#include <unistd.h> /* ssize_t */
typedef long ssize_t;	
#ifdef __cplusplus
extern "C" {
#endif
  extern ssize_t getdelim(char **lineptr, size_t *n, int delimiter, FILE *fp);
  extern ssize_t getline (char **lineptr, size_t *n, FILE *stream);
#ifdef __cplusplus
}
#endif
#endif

#endif // _MACPORTS_STDIO_H_
