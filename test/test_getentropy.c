/*
 * Simple test harness and benchmark for MT Arc4Random
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/random.h>
#include <assert.h>

int
main()
{
  char buf[8];
  int r = getentropy(&buf, sizeof buf);
  assert(r == 0);
  printf( "getentropy : %i\n", r );
  return r;
}
