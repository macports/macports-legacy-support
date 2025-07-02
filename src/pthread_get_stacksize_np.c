/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_PTHREAD_GET_STACKSIZE_NP_FIX__

#include <pthread.h>
#include <sys/resource.h>
#include <stdlib.h>

#include "util.h"

#if __MPLS_TARGET_OSVER >= 1090
/* private system call available on OS X Mavericks (version 10.9) and later */
/* see https://github.com/apple-oss-distributions/libpthread/blob/ba8e1488a0e6848b710c5daad2e226f66cfed656/private/pthread/private.h#L34 */
pthread_t pthread_main_thread_np(void);
#endif

#define kMaxThreadStackSize 0x40000000 /* from LLVM: 1 << 30 or 1Gb */

size_t pthread_get_stacksize_np(pthread_t t) {
#if __MPLS_TARGET_OSVER >= 1090
     int is_main_thread = pthread_equal(t, pthread_main_thread_np());
#else
     /* taken from Apple Libc */
     /* see https://github.com/apple-oss-distributions/Libc/blob/224a8105d573730ae33f474ae5b63b113123aee4/pthreads/pthread.c#L1167 */
     /* for _PTHREAD_CREATE_PARENT, see https://github.com/apple-oss-distributions/Libc/blob/224a8105d573730ae33f474ae5b63b113123aee4/pthreads/pthread_internals.h#L647C9-L647C31 */
     /* for pthread_t, see https://github.com/apple-oss-distributions/Libc/blob/224a8105d573730ae33f474ae5b63b113123aee4/pthreads/pthread_internals.h#L107 */
     /* for pthread_lock_t, see https://github.com/apple-oss-distributions/Libc/blob/224a8105d573730ae33f474ae5b63b113123aee4/pthreads/pthread_machdep.h#L214C13-L214C27 */
     struct
     {
          long sig;
          struct __darwin_pthread_handler_rec *cleanup_stack;
          int lock;
          __int32_t detached:8,
                    inherit:8,
                    policy:8,
                    freeStackOnExit:1,
                    newstyle:1,
                    kernalloc:1,
                    schedset:1,
                    wqthread:1,
                    pad:3;
                    char opaque[__PTHREAD_SIZE__-sizeof(int)-sizeof(__int32_t)];
     } *thread = (void*) t;
     int is_main_thread = ((thread->detached & 4) == 4);
#endif
    if ( is_main_thread ) {
        /* use LLVM workaround */
        /* see https://github.com/llvm/llvm-project/blob/617a15a9eac96088ae5e9134248d8236e34b91b1/compiler-rt/lib/sanitizer_common/sanitizer_mac.cpp#L414 */
       /* OpenJDK also has a workaround */
       /* see https://github.com/openjdk/jdk/blob/e833bfc8ac6104522d037e7eb300f5aa112688bb/src/hotspot/os_cpu/bsd_x86/os_bsd_x86.cpp#L715 */
        struct rlimit limit;
        if( getrlimit(RLIMIT_STACK, &limit) ) {
             exit(EXIT_FAILURE);
        }
        if( limit.rlim_cur < kMaxThreadStackSize ) {
             return limit.rlim_cur;
        } else {
             return kMaxThreadStackSize;
        }
    } else {
        /* bug only affects main thread */
        GET_OS_FUNC(pthread_get_stacksize_np)
        return (*os_pthread_get_stacksize_np)(t);
    }
}

#endif /* __MPLS_LIB_SUPPORT_PTHREAD_GET_STACKSIZE_NP_FIX__ */
