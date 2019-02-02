/* MP support header */
#include "MacportsLegacySupport.h"


#if __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* redefine the original sysconf */
#undef sysconf
#define sysconf(a) sysconf_orig(a)

#endif /*__ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__*/



#include_next <unistd.h>



#if __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__

/* and now define sysconf as our new wrapped function */
#undef sysconf
#include "MacportsLegacyWrappers/sysconf_support.h"

#ifdef __cplusplus
extern "C" {
#endif
extern long sysconf(int) __MP_LEGACY_WRAPPER_ALIAS(sysconf);
#ifdef __cplusplus
}
#endif

#endif /* __ENABLE_MP_LEGACY_SUPPORT_SYSCONF_WRAP__ */
