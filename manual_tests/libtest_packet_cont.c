/*
 * Version of test_packet with secret SO_TIMESTAMP_CONTINUOUS enabled.
 *
 * This exists in 10.14+ kernels, but isn't defined for userspace.
 */

/* Definitions from xnu/bsd/sys/socket_private.h */
/* With #ifndefs just in case some SDK decides to define them */
#ifndef SO_TIMESTAMP_CONTINUOUS
#define SO_TIMESTAMP_CONTINUOUS 0x40000 /* Continuous monotonic timestamp on rcvd dgram */
#endif
#ifndef SCM_TIMESTAMP_CONTINUOUS
#define SCM_TIMESTAMP_CONTINUOUS        0x07    /* timestamp (uint64_t) */
#endif

#include "../test/test_packet.c"
