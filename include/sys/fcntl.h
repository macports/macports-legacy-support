#ifndef _MACPORTS_SYSFCNTL_H_
#define _MACPORTS_SYSFCNTL_H_

// Include the primary system wchar.h
#include_next <sys/fcntl.h>

// replace missing O_CLOEXIT definition with 0, which works
// but does not replace the full function of that flag
// this is the commonly done fix in MacPorts (see gtk3, for example)
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#endif /* _MACPORTS_SYSFCNTL_H_ */
