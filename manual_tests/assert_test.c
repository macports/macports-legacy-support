#include <assert.h>
#include <stdio.h>

int main()
{
  static_assert(1, "true");
  printf("static_assert compiled successfully in C11 mode\n\n");
}
