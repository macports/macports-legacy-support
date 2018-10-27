#ifndef _MACPORTS_MMAN_H_
#define _MACPORTS_MMAN_H_

// Include the primary system sys/mman.h
#include_next <sys/mman.h>

// MAP_ANONYMOUS only exists on 10.11+
// Prior to that it was called MAP_ANON
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif /* _MACPORTS_MMAN_H_ */
