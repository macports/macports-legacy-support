/* MP support header */
#include "MacportsLegacySupport.h"


#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* redefine the original sysconf */
#undef sysconf
#define sysconf sysconf_orig

#endif /*__MP_LEGACY_SUPPORT_SYSCONF_WRAP__*/



#include_next <unistd.h>



#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* and now define sysconf as our new wrapped function */
#undef sysconf
#define sysconf macports_legacy_sysconf

#ifndef _SC_NPROCESSORS_CONF
#define _SC_NPROCESSORS_CONF 57
#endif

#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN 58
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern long macports_legacy_sysconf(int);
#ifdef __cplusplus
}
#endif

#endif /*__MP_LEGACY_SUPPORT_SYSCONF_WRAP__*/
