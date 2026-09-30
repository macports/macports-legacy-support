#ifndef _MACPORTS_SYS_SIGNAL_H_
#define _MACPORTS_SYS_SIGNAL_H_

#include "MacportsLegacySupport.h"
#include <_macports_extras/sdkversion.h>

#include_next <sys/signal.h>

/* Newer GCC compilers use C23 as the default. On Tiger, signal.h is not C23
compliant without defining specifics macro that limit what other function
prototypes are available. See:
https://gcc.gnu.org/gcc-15/porting_to.html#c23-fn-decls-without-parameters */

#if __MPLS_SDK_MAJOR < 1050

#undef SIG_DFL
#undef SIG_IGN
#undef SIG_HOLD
#undef SIG_ERR

#if defined(_ANSI_SOURCE) || defined(_POSIX_C_SOURCE) || defined(__cplusplus) \
|| (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
/*
 * Language spec sez we must list exactly one parameter, even though we
 * actually supply three.  Ugh!
 * SIG_HOLD is chosen to avoid KERN_SIG_* values in <sys/signalvar.h>
 */
#define	SIG_DFL		(void (*)(int))0
#define	SIG_IGN		(void (*)(int))1
#define	SIG_HOLD	(void (*)(int))5
#define	SIG_ERR		((void (*)(int))-1)
#else
/* DO NOT REMOVE THE COMMENTED OUT int: fixincludes needs to see them */
#define	SIG_DFL		(void (*)(/*int*/))0
#define	SIG_IGN		(void (*)(/*int*/))1
#define	SIG_HOLD	(void (*)(/*int*/))5
#define	SIG_ERR		((void (*)(/*int*/))-1)
#endif

#endif /* __MPLS_SDK_MAJOR */

#endif /* _MACPORTS_SYS_SIGNAL_H_ */

