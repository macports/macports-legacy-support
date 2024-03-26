#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>

int main()
{
     void (*fptr)();

     // void __bzero(void *s, size_t n);
     fptr = dlsym(RTLD_DEFAULT, "__bzero");
     assert( fptr!=0 && "__bzero symbol does not exist");

     // int dirfd(DIR *dirp);
     fptr = dlsym(RTLD_DEFAULT, "dirfd");
     assert( fptr!=0 && "dirfd symbol does not exist");

#if  __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050 && !defined(__arm64__)
     // int fstatat$INODE64(int dirfd, const char *pathname, struct stat64 *buf, int flags);
     fptr = dlsym(RTLD_DEFAULT, "fstatat$INODE64");
     assert( fptr!=0 && "fstatat$INODE64 symbol does not exist");
#endif

     printf("All symbol aliases found\n");

     return 0;
}
