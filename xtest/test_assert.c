#include <assert.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

#if __STDC_VERSION__ >= 201100
  static_assert(1, "true");
  printf("static_assert compiled successfully in C11 mode\n\n");
#else
  printf("static_assert is unavailable before C11, __STDC_VERSION__ = %d\n",
         (int) __STDC_VERSION__);
#endif
  return 0;
}
