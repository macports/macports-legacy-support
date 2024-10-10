/*
 * Test for CMSG_DATA() macro
 */

#include <stdio.h>
#include <sys/socket.h>
#include <assert.h>

const static struct {
  struct cmsghdr  hdr;
  unsigned char   data[4];
} test = {{0}};

int
main()
{
  const unsigned char *testb = (const unsigned char *) &test;
  int real_ofs = &test.data[0] - testb;
  int macro_ofs = CMSG_DATA(&test) - testb;

  assert(macro_ofs == real_ofs);
  printf("CMSG_DATA offset = %d\n", macro_ofs);
  return 0;
}
